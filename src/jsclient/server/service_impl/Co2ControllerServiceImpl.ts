import { injectable, postConstruct } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService from "server/service/Co2ControllerService";
import { combineLatest, timer } from "rxjs";
import { map, distinctUntilChanged, startWith, throttleTime } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";
import { isPresent } from "../misc/isPresent";
import config from "server/config";
import { Timestamp } from "server/misc/Timestamp";
import { Subscriptions } from "server/misc/Subscriptions";
import TimeService from "server/service/TimeService";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";

// TODO: Use prediction

// TODO: Gradually change closing point:
// TODO:   From 8:00 to 10:00 it must be change over time from 7.2 to 6.9 using exponential formula
// TODO:   From 10 to 20:00 it must be change over time from 7.2 to 6.8 using exponential formula

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 60_000;

// Give value actuation a chance....
const THROTTLE_TIME_MS = 5_000;

// Don't allow it be open for too long to avoid overheating and stuff
const CO2_MAX_OPEN_MINUTES = 30;

// Decide whether we must turn ph on or not
function isCo2Required(
    state: {
        ph?: number | null,
        co2ValveOpen?: boolean | null,
        co2ValveOpenSeconds: number
    }
): boolean {
    if (!isPresent(state.ph) || !isPresent(state.co2ValveOpen)) {
        return false;
    }

    if (state.co2ValveOpenSeconds > (CO2_MAX_OPEN_MINUTES * 60)) {
        return false;
    }

    if (state.co2ValveOpen) {
        return state.ph > config.phController.phToTurnOff;
    } else {
        return state.ph >= config.phController.phToTurnOn;
    }
}

@injectable()
export default class Co2ControllerServiceImpl extends Co2ControllerService {
    private readonly _subs = new Subscriptions();
    private _co2ValveOpenT?: Timestamp;

    constructor(
        private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService,
        private readonly _timeService: TimeService
    ) {
        super();
    }

    @postConstruct()
    _init() {
        // Detect how long CO2 valve has been open (we just remember when CO2 valve was opened)
        this._subs.add(
            this._avrService.avrState$.subscribe(avrState => {
                if (this._co2ValveOpenT && !avrState.co2ValveOpen) {
                    // Going from open to closed
                    this._co2ValveOpenT = undefined;
                } else if (!this._co2ValveOpenT && avrState.co2ValveOpen) {
                    // Going from closed to open. Remember this moment
                    this._co2ValveOpenT = this._timeService.nowTimestamp();
                }
            })
        );

        // State changer
        this._subs.add(
            combineLatest([
                this._phSensorService.ph$.pipe(
                    map(ph => ph?.value600s),
                    distinctUntilChanged(),
                    startWith(undefined)
                ),
                this._avrService.avrState$.pipe(
                    map(avrState => avrState.co2ValveOpen),
                    distinctUntilChanged(),
                    startWith(undefined)
                ),
                timer(0, SEND_REQUIREMENTS_TO_AVR_EVERY_MS)
            ]).pipe(
                map(([ph, co2ValveOpen]) => {
                    const co2ValveOpenSeconds = this._co2ValveOpenT ? getElapsedSecondsSince(this._co2ValveOpenT) : 0;

                    return isCo2Required({
                        ph,
                        co2ValveOpen,
                        co2ValveOpenSeconds
                    });
                }),
                throttleTime(THROTTLE_TIME_MS)
            ).subscribe(required => {
                this._avrService.setCo2RequiredValveOpenState(
                    required ? Co2ValveOpenState.Open : Co2ValveOpenState.Closed
                );
            })
        );
    }

    // Used in unit testing
    _destroy(): void {
        this._subs.unsubscribeAll();
    }
}
