import { injectable, postConstruct, optional, inject } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService, { PhControlRange } from "server/service/Co2ControllerService";
import { combineLatest, timer, of, SchedulerLike } from "rxjs";
import { map, distinctUntilChanged, startWith, throttleTime, skip, timeoutWith, repeat, share } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";
import { isPresent } from "../misc/isPresent";
import config, { PhControllerConfig } from "server/config";
import { Timestamp } from "server/misc/Timestamp";
import { Subscriptions } from "server/misc/Subscriptions";
import TimeService from "server/service/TimeService";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";
import PhPredictionService from "server/service/PhPredictionService";
import RandomNumberService from "server/service/RandomNumberService";
import logger from "server/logger";

// TODO: Move constants to config!
// TODO: Implement safety check on ph60s to limit it to 6.5!

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 60_000;

// Give value-actuation a chance....
const THROTTLE_TIME_MS = 5_000;

// Don't allow it be open for too long to avoid overheating/"over-ventilation" and stuff
const CO2_MAX_OPEN_MINUTES = 30;

// Don't believe in predictions based upon first 30 seconds! It might be plain wrong.
// (we also have resolutions of 15 seconds based upon data from grafana)
const CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS = 2;

// Timeout for values form upstream observables
const OBS_TIMEOUT_MS = 60_000;

// Maximum allowed percentage (0...1) of changes relative to controlled margin between predictions to look real. 
const MAX_PERCENTAGE_OF_CHANGE_BETWEEN_PREDICTIONS = 0.6;

// Solution (a, b, c , d) for the following equations, given f(t1) = ph1, f(t2) = ph2, f(t3) = ph3:
// Maxima:
//   optimize(solve([a * e^(100 / t1) + b = ph1, a * e^(100 / t2) + b = ph2], [a, b]));
//   optimize(solve([c * e^(t2 / 2) + d = ph2, c * e^(t3 / 2) + d = ph3], [c, d]));
export interface MinPhEquationParams {
    a: number;
    b: number;
    c: number;
    d: number;
}

export function calcMinPhEquationParams(phControllerConfig: PhControllerConfig): MinPhEquationParams {
    // See above for a way to find this in 'Maxima'.
    const t1 = phControllerConfig.dayPrepareHour;
    const t2 = phControllerConfig.dayStartHour;
    const t3 = phControllerConfig.dayEndHour;
    const ph1 = phControllerConfig.dayEndPh;
    const ph2 = phControllerConfig.dayStartPh;
    const ph3 = phControllerConfig.dayEndPh;

    const l1 = Math.exp(100 / t1);
    const l2 = Math.exp(100 / t2);
    const l3 = 1 / (l1 - l2);
    const a = l3 * (ph1 - ph2);
    const b = -l3 * (l2 * ph1 - l1 * ph2);

    const m1 = Math.exp(t2 / 2);
    const m2 = Math.exp(t3 / 2);
    const m3 = 1 / (m1 - m2);
    const c = m3 * (ph2 - ph3);
    const d = -m3 * (m2 * ph2 - m1 * ph3);

    return { a, b, c, d };
}

export function calcMinPh(config: PhControllerConfig, solution: MinPhEquationParams, hour: number): number | undefined {
    // See above (about Maxima stuff)

    if (hour < config.dayPrepareHour || hour > config.dayEndHour) {
        return undefined;
    }

    if (hour < config.dayStartHour) {
        return solution.a * Math.exp(100 / hour) + solution.b;
    }

    return solution.c * Math.exp(hour / 2) + solution.d;
}

const minPhEquationSolution = calcMinPhEquationParams(config.phController);

function calcCurrentMinPh(): number | undefined {
    const date = new Date();
    const hour = date.getHours() + date.getMinutes() / 60 + date.getSeconds() / 3600;
    return calcMinPh(config.phController, minPhEquationSolution, hour);
}

// Decide whether we must turn ph on or not
function isCo2Required(
    state: {
        ph600?: number | null,
        ph60?: number | null,
        co2ValveOpen?: boolean | null,
        co2ValveOpenSeconds: number,
        predictedMinPh?: number,
        previouslyPredictedMinPh?: number,
        co2MaxOpenSecondsForExplorationReason: number
    }
): [boolean, string] {
    const phToTurnOff = calcCurrentMinPh();

    if (!isPresent(state.ph600) || !isPresent(state.ph60) || !isPresent(state.co2ValveOpen) || !isPresent(phToTurnOff)) {
        return [false, `Missing required info: ph600=${state.ph600}, ph60=${state.ph60}, co2ValveOpen=${state.co2ValveOpen}, phToTurnOff=${phToTurnOff}`];
    }

    const phToTurnOn = phToTurnOff + config.phController.phTurnOnOffMargin;

    if (state.co2ValveOpen) {
        // CO2 Valve is currently open (CO2 is supplied into the tank)

        // Hard limit on number of seconds
        if (state.co2ValveOpenSeconds > (CO2_MAX_OPEN_MINUTES * 60)) {
            return [false, `Hard limit on open seconds (${state.co2ValveOpenSeconds} secs). Limit ${CO2_MAX_OPEN_MINUTES} minutes`];
        }

        // Give our AI some exploration space.. close the valve at some random times.
        // This helps us have more diverse data. IF we close too soon, it's not a problem
        // because the valve will soon be opened again.
        if (state.co2MaxOpenSecondsForExplorationReason && state.co2ValveOpenSeconds > state.co2MaxOpenSecondsForExplorationReason) {
            return [false, `Exploration`];
        }

        // Check against safe ph
        if (state.ph600 <= config.phController.minSafePh600) {
            // Close
            return [false, `Ph600 safe ph limit is ${config.phController.minSafePh600}, current ph600 is ${state.ph600}`];
        }

        if (state.ph60 <= config.phController.minSafePh60) {
            // Close
            return [false, `Ph60 safe ph limit is ${config.phController.minSafePh60}, current ph60 is ${state.ph60}`];
        }

        // Check predicted ph if present
        if (state.predictedMinPh && state.predictedMinPh <= phToTurnOff) {
            // Avoid trusting pessimistic predictions based upon insufficient data
            if (state.co2ValveOpenSeconds > CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS) {
                // Turn of, because we predict that if we turn it off now, then
                // we will go under the limit or to the min-level-limit anyway, so it is best
                // to turn the valve off now and not wait until it will be worse
                return [false, `PH Predicted to reach ${state.predictedMinPh}, while phToTurnOff=${phToTurnOff}`];
            }
        }

        // Check predicted ph change if present
        if (state.predictedMinPh && state.previouslyPredictedMinPh) {
            const absPredictionChange = Math.abs(state.previouslyPredictedMinPh - state.predictedMinPh);
            const changeLimit = config.phController.phTurnOnOffMargin * MAX_PERCENTAGE_OF_CHANGE_BETWEEN_PREDICTIONS;

            // Avoid trusting pessimistic predictions based upon insufficient data
            if (state.co2ValveOpenSeconds > CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS && absPredictionChange >= changeLimit) {
                return [false, `PH Predicted to reach ${state.predictedMinPh}, previous prediction was ${state.previouslyPredictedMinPh}, absChange=${absPredictionChange}, change limit=${changeLimit}`];
            }
        }

        // Keep open if current ph is greater than the configured ph-to-turn-off
        return [state.ph600 > phToTurnOff, `ph600=${state.ph600}, phToTurnOff=${phToTurnOff}`];
    } else {
        // CO2 Valve is currently closed (CO2 is NOT supplied into the tank)

        // Open if current ph is greater or equal to the configured ph-to-turn-on
        return [state.ph600 >= phToTurnOn, `ph600=${state.ph600}, phToTurnOn=${phToTurnOn}`];
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

        // State changer - - - -  - - - - - - - - - - - -  - - - - - - - - - - - - - - 
        // TODO: Make a custom pipe or something to avoid this boring repeating pattern!

        const minClosingPhPrediction$ =
            this._phPredictionService.minClosingPhPrediction$.pipe(
                skip(1), // to avoid hot value
                timeoutWith(OBS_TIMEOUT_MS, of(undefined), this._scheduler), // protect against stale data
                repeat(), // resubscribe in case of timeout or error
                map(prediction =>
                    isPresent(prediction) && !prediction.valveIsAlreadyClosed ? prediction.predictedMinPh : undefined
                ),
                distinctUntilChanged(),
                share()
            );

        // TODO: Make a custom pipe or something to avoid this boring repeating pattern!
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
                this._phSensorService.ph$.pipe(
                    skip(1), // to avoid hot value
                    timeoutWith(OBS_TIMEOUT_MS, of(undefined), this._scheduler), // protect against stale data
                    repeat(), // resubscribe in case of timeout or error
                    map(ph => ph?.value60s),
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
                minClosingPhPrediction$.pipe(skip(1), startWith(undefined)),
                minClosingPhPrediction$.pipe(startWith(undefined)),
                timer(0, SEND_REQUIREMENTS_TO_AVR_EVERY_MS)
            ]).pipe(
                map(([ph600, ph60, co2ValveOpen, previouslyPredictedMinPh, predictedMinPh]) => {
                    const co2ValveOpenSeconds = this._co2ValveOpenT ? getElapsedSecondsSince(this._co2ValveOpenT) : 0;

                    const [required, msg] = isCo2Required({
                        ph600,
                        ph60,
                        co2ValveOpen,
                        co2ValveOpenSeconds,
                        previouslyPredictedMinPh,
                        predictedMinPh,
                        co2MaxOpenSecondsForExplorationReason: this._co2MaxOpenSecondsForExplorationReason
                    });

                    return { required, msg, co2ValveOpen };
                }),
                throttleTime(THROTTLE_TIME_MS)
            ).subscribe(decisionInfo => {
                const { required, msg, co2ValveOpen } = decisionInfo;

                if (co2ValveOpen != required) {
                    logger.info(`Co2Controller: Required=${required}. Msg: ${msg}`);
                }

                this._avrService.setCo2RequiredValveOpenState(
                    required ? Co2ValveOpenState.Open : Co2ValveOpenState.Closed
                );
            })
        );
    }

    getPhControlRange(): PhControlRange {
        const minPh = calcCurrentMinPh();
        return {
            phToTurnOff: minPh,
            phToTurnOn: isPresent(minPh) ? (minPh + config.phController.phTurnOnOffMargin) : undefined
        };
    }

    // Used in unit testing
    _destroy(): void {
        this._subs.unsubscribeAll();
    }
}
