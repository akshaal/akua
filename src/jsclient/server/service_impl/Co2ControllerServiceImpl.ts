import { injectable, postConstruct, optional, inject } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService from "server/service/Co2ControllerService";
import { combineLatest, timer, of, SchedulerLike } from "rxjs";
import { map, distinctUntilChanged, startWith, throttleTime, skip, timeoutWith, repeat } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";
import { isPresent } from "../misc/isPresent";
import config from "server/config";
import { Timestamp } from "server/misc/Timestamp";
import { Subscriptions } from "server/misc/Subscriptions";
import TimeService from "server/service/TimeService";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";
import PhPredictionService from "server/service/PhPredictionService";
import RandomNumberService from "server/service/RandomNumberService";

// TODO: Move constants to config!

// TODO: Gradually change closing point:
// TODO:   From 8:00 to 10:00 it must be change over time from 7.2 to 6.9 using exponential formula
// TODO:   From 10 to 20:00 it must be change over time from 7.2 to 6.8 using exponential formula

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 60_000;

// Give value actuation a chance....
const THROTTLE_TIME_MS = 5_000;

// Don't allow it be open for too long to avoid overheating/"over-ventilation" and stuff
const CO2_MAX_OPEN_MINUTES = 30;

// Don't believe in predictions based upon first 30 seconds! It might be plain wrong.
// (we also have resolutions of 15 seconds based upon data from grafana)
const CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS = 2;

// Timeout for values form upstream observables
const OBS_TIMEOUT_MS = 60_000;

// Decide whether we must turn ph on or not
function isCo2Required(
    state: {
        ph?: number | null,
        co2ValveOpen?: boolean | null,
        co2ValveOpenSeconds: number,
        predictedMinPh?: number,
        co2MaxOpenSecondsForExplorationReason: number
    }
): boolean {
    if (!isPresent(state.ph) || !isPresent(state.co2ValveOpen)) {
        return false;
    }

    if (state.co2ValveOpen) {
        // CO2 Valve is currently open (CO2 is supplied into the tank)

        // Hard limit on number of seconds
        if (state.co2ValveOpenSeconds > (CO2_MAX_OPEN_MINUTES * 60)) {
            return false;
        }

        // Give our AI some exploration space.. close the valve at some random times.
        // This helps us have more diverse data. IF we close too soon, it's not a problem
        // because the valve will soon be opened again.
        if (state.co2MaxOpenSecondsForExplorationReason && state.co2ValveOpenSeconds > state.co2MaxOpenSecondsForExplorationReason) {
            return false;
        }

        // Check predicted ph if present
        if (state.predictedMinPh && state.predictedMinPh <= config.phController.phToTurnOff) {
            // Avoid trusting pessimistic predictions based upon insufficient data
            if (state.co2ValveOpenSeconds > CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS) {
                // Turn of, because we predict that if we turn it off now, then
                // we will go under the limit or to the min-level-limit anyway, so it is best
                // to turn the valve off now and not wait until it will be worse
                return false; // CO2 is no longer required
            }
        }

        // Keep open if current ph is greater than the configured ph-to-turn-off
        return state.ph > config.phController.phToTurnOff;
    } else {
        // CO2 Valve is currently closed (CO2 is NOT supplied into the tank)

        // Open if current ph is greater or equal to the configured ph-to-turn-on
        return state.ph >= config.phController.phToTurnOn;
    }
}

@injectable()
export default class Co2ControllerServiceImpl extends Co2ControllerService {
    private readonly _subs = new Subscriptions();
    private _co2ValveOpenT?: Timestamp;

    // How many seconds we can keep co2 valve open. This value is reconfigured
    // each time we open CO2 valve. See _init method.
    private _co2MaxOpenSecondsForExplorationReason: number = 0;

    constructor(
        private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService,
        private readonly _timeService: TimeService,
        private readonly _randomNumberService: RandomNumberService,
        private readonly _phPredictionService: PhPredictionService,
        @optional() @inject("scheduler") private readonly _scheduler: SchedulerLike
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

                    // Reconfigure the limit to some random value
                    this._co2MaxOpenSecondsForExplorationReason = 1 + this._randomNumberService.next() * CO2_MAX_OPEN_MINUTES * 60;
                }
            })
        );

        // State changer
        this._subs.add(
            combineLatest([
                this._phSensorService.ph$.pipe(
                    skip(1), // to avoid hot value
                    timeoutWith(OBS_TIMEOUT_MS, of(undefined), this._scheduler), // protect against stale data
                    repeat(), // resubscribe in case of timeout or error
                    map(ph => ph?.value600s),
                    distinctUntilChanged(),
                    startWith(undefined)
                ),
                this._avrService.avrState$.pipe(
                    skip(1), // to avoid hot value
                    timeoutWith(OBS_TIMEOUT_MS, of(undefined), this._scheduler), // protect against stale data
                    repeat(), // resubscribe in case of timeout or error
                    map(avrState => avrState?.co2ValveOpen),
                    distinctUntilChanged(),
                    startWith(undefined)
                ),
                this._phPredictionService.minClosingPhPrediction$.pipe(
                    skip(1), // to avoid hot value
                    timeoutWith(OBS_TIMEOUT_MS, of(undefined), this._scheduler), // protect against stale data
                    repeat(), // resubscribe in case of timeout or error
                    map(prediction =>
                        isPresent(prediction) && !prediction.valveIsAlreadyClosed ? prediction.predictedMinPh : undefined
                    ),
                    distinctUntilChanged(),
                    startWith(undefined)
                ),
                timer(0, SEND_REQUIREMENTS_TO_AVR_EVERY_MS)
            ]).pipe(
                map(([ph, co2ValveOpen, predictedMinPh]) => {
                    const co2ValveOpenSeconds = this._co2ValveOpenT ? getElapsedSecondsSince(this._co2ValveOpenT) : 0;

                    return isCo2Required({
                        ph,
                        co2ValveOpen,
                        co2ValveOpenSeconds,
                        predictedMinPh,
                        co2MaxOpenSecondsForExplorationReason: this._co2MaxOpenSecondsForExplorationReason
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
