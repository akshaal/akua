import { injectable, postConstruct } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService from "server/service/Co2ControllerService";
import { combineLatest, timer } from "rxjs";
import { map, distinctUntilChanged, startWith, throttleTime } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";
import { isPresent } from "../misc/isPresent";
import config from "server/config";

// TODO: Use prediction
// TODO: Don't let CO2 valve be open longe than 1 hour
// TODO: Gradually change turning point from 6.8 to 7.2 during the day

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 60_000;

// Give value actuation a chance....
const THROTTLE_TIME_MS = 5_000;

// Decide whether we must turn ph on or not
function isCo2Required(state: { ph?: number | null, co2ValveOpen?: boolean | null }): boolean {
    if (!isPresent(state.ph) || !isPresent(state.co2ValveOpen)) {
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
    constructor(private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService) {
        super();
    }

    @postConstruct()
    _init() {
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
            map(([ph, co2ValveOpen]) => isCo2Required({ ph, co2ValveOpen })),
            throttleTime(THROTTLE_TIME_MS)
        ).subscribe(required => {
            this._avrService.setCo2RequiredValveOpenState(
                required ? Co2ValveOpenState.Open : Co2ValveOpenState.Closed
            );
        });
    }
}
