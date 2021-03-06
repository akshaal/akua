import { injectable, postConstruct, optional, inject } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService, { PhControlRange } from "server/service/Co2ControllerService";
import { combineLatest, timer, of, SchedulerLike, pipe, Observable, UnaryFunction } from "rxjs";
import { map, distinctUntilChanged, startWith, skip, timeoutWith, repeat, share, throttle, pairwise } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";
import { isPresent } from "../misc/isPresent";
import { Timestamp } from "server/misc/Timestamp";
import { Subscriptions } from "server/misc/Subscriptions";
import TimeService from "server/service/TimeService";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";
import PhPredictionService from "server/service/PhPredictionService";
import RandomNumberService from "server/service/RandomNumberService";
import logger from "server/logger";
import ConfigService, { PhControllerConfig } from "server/service/ConfigService";

// TODO: Unit tests!
// TODO: Move constants to config!

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 10_000;

// Give value-actuation a chance.... be unpredictable to avoid patterns.. at least between restarts
const THROTTLE_TIME_MS = 3_000 + Math.random() * 2_000;

// Don't allow it be open for too long to avoid overheating/"over-ventilation" and stuff
const CO2_MAX_OPEN_MINUTES = 15; // TODO: Temporary

// Don't believe in predictions based upon first 30 seconds! It might be plain wrong.
// (we also have resolutions of 15 seconds based upon data from grafana)
const CO2_MIN_OPEN_SECONDS_TO_TRUST_PREDICTIONS = 2;

// Timeout for values form upstream observables
const OBS_TIMEOUT_MS = 60_000;

// Maximum allowed percentage (0...1) of changes relative to controlled margin between predictions to look real. 
const MAX_PERCENTAGE_OF_CHANGE_BETWEEN_PREDICTIONS = 0.4;

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

// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////

export function calcMinPhEquationParams(params: { phControllerConfig: PhControllerConfig, altDay: boolean }): MinPhEquationParams {
    const { altDay, phControllerConfig } = params;

    // See above for a way to find this in 'Maxima'.
    const t1 = altDay ? phControllerConfig.altDayPrepareHour : phControllerConfig.normDayPrepareHour;
    const t2 = altDay ? phControllerConfig.altDayStartHour : phControllerConfig.normDayStartHour;
    const t3 = altDay ? phControllerConfig.altDayEndHour : phControllerConfig.normDayEndHour;
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

// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////

// (exported in order to test)

export function calcMinPh(params: {
    config: PhControllerConfig,
    solution: MinPhEquationParams,
    hour: number,
    altDay: boolean
}): number | undefined {
    const { config, solution, hour, altDay } = params;

    // See above (about Maxima stuff)

    const dayPrepareHour = altDay ? config.altDayPrepareHour : config.normDayPrepareHour;
    const dayStartHour = altDay ? config.altDayStartHour : config.normDayStartHour;
    const dayEndHour = altDay ? config.altDayEndHour : config.normDayEndHour;

    if (hour < dayPrepareHour || hour > dayEndHour) {
        return undefined;
    }

    if (hour < dayStartHour) {
        return solution.a * Math.exp(100 / hour) + solution.b;
    }

    return solution.c * Math.exp(hour / 2) + solution.d;
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////

// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////

function asSourceOfDecision<T, R>(scheduler: SchedulerLike, mapper: (orig?: T) => R): UnaryFunction<Observable<T>, Observable<R>> {
    return pipe(
        skip(1), // to avoid hot value
        timeoutWith(OBS_TIMEOUT_MS, of(undefined), scheduler), // protect against stale data
        repeat(), // resubscribe in case of timeout or error
        map(mapper),
        distinctUntilChanged(),
        share()
    );
}

// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////////////////////////////////////

@injectable()
export default class Co2ControllerServiceImpl extends Co2ControllerService {
    private readonly _minPhEquationSolutionForNormDay = calcMinPhEquationParams({
        phControllerConfig: this._configService.config.phController,
        altDay: false
    });
    
    private readonly _minPhEquationSolutionForAltDay = calcMinPhEquationParams({
        phControllerConfig: this._configService.config.phController,
        altDay: true
    });
        
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
        private readonly _configService: ConfigService,
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

        const ph600$: Observable<number | undefined | null> =
            this._phSensorService.ph$.pipe(asSourceOfDecision(this._scheduler, ph => ph?.value600s));

        const ph60$: Observable<number | undefined | null> =
            this._phSensorService.ph$.pipe(asSourceOfDecision(this._scheduler, ph => ph?.value60s));

        const co2ValveOpen$: Observable<boolean | undefined> =
            this._avrService.avrState$.pipe(asSourceOfDecision(this._scheduler, avrState => avrState?.co2ValveOpen));

        const minClosingPhPrediction$: Observable<number | undefined> =
            this._phPredictionService.minClosingPhPrediction$.pipe(
                asSourceOfDecision(this._scheduler, prediction =>
                    isPresent(prediction) && !prediction.valveIsAlreadyClosed ? prediction.predictedMinPh : undefined
                )
            );

        const co2Decision$: Observable<{ required: boolean, msg: string, co2ValveOpen: boolean | undefined }> =
            combineLatest([
                ph600$.pipe(startWith(undefined)),
                ph60$.pipe(startWith(undefined)),
                co2ValveOpen$.pipe(startWith(undefined)),
                minClosingPhPrediction$.pipe(startWith(undefined, undefined), pairwise()),
                timer(0, SEND_REQUIREMENTS_TO_AVR_EVERY_MS)
            ]).pipe(
                map(([ph600, ph60, co2ValveOpen, [previouslyPredictedMinPh, predictedMinPh]]) => {
                    const now = this._timeService.nowTimestamp();
                    const co2ValveOpenSeconds = this._co2ValveOpenT ? getElapsedSecondsSince({ now, since: this._co2ValveOpenT }) : 0;

                    const [required, msg] = this._isCo2Required({
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
                share()
            );

        this._subs.add(
            co2Decision$.pipe(
                pairwise(),
                throttle(([prevCo2Decision, co2Decision]) => {
                    if (prevCo2Decision.required == co2Decision.required) {
                        // Don't throttle if no state change
                        return timer(1, this._scheduler);
                    } else {
                        // Ignore consequent state changes after this one.
                        // This way we avoid bouncing decisions while previous decision is not yet actuated
                        return timer(THROTTLE_TIME_MS, this._scheduler);
                    }
                })
            ).subscribe(([_, co2Decision]) => {
                const { required, msg, co2ValveOpen } = co2Decision;

                if (co2ValveOpen && !required) {
                    logger.info(`Co2Controller: Decided to turn off CO2: ${msg}`);
                }

                this._avrService.setCo2RequiredValveOpenState(
                    required ? Co2ValveOpenState.Open : Co2ValveOpenState.Closed
                );
            })
        );
    }

    getPhControlRange(): PhControlRange {
        const minPh = this._calcCurrentMinPh();
        return {
            phToTurnOff: minPh,
            phToTurnOn: isPresent(minPh) ? (minPh + this._configService.config.phController.phTurnOnOffMargin) : undefined
        };
    }

    // Used in unit testing
    _destroy(): void {
        this._subs.unsubscribeAll();
    }

    // Decide whether we must turn ph on or not
    private _isCo2Required(
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
        const phToTurnOff = this._calcCurrentMinPh();

        if (!isPresent(state.ph600) || !isPresent(state.ph60) || !isPresent(state.co2ValveOpen) || !isPresent(phToTurnOff)) {
            return [false, `Missing required info: ph600=${state.ph600}, ph60=${state.ph60}, co2ValveOpen=${state.co2ValveOpen}, phToTurnOff=${phToTurnOff}`];
        }

        const phToTurnOn = phToTurnOff + this._configService.config.phController.phTurnOnOffMargin;

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
            if (state.ph600 <= this._configService.config.phController.minSafePh600) {
                // Close
                return [false, `Ph600 safe ph limit is ${this._configService.config.phController.minSafePh600}, current ph600 is ${state.ph600}`];
            }

            if (state.ph60 <= this._configService.config.phController.minSafePh60) {
                // Close
                return [false, `Ph60 safe ph limit is ${this._configService.config.phController.minSafePh60}, current ph60 is ${state.ph60}`];
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
                const changeLimit = this._configService.config.phController.phTurnOnOffMargin * MAX_PERCENTAGE_OF_CHANGE_BETWEEN_PREDICTIONS;

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

    private _calcCurrentMinPh(): number | undefined {
        const altDay = this._configService.config.aquaEnv.alternativeDay;

        const date = new Date();
        const hour = date.getHours() + date.getMinutes() / 60 + date.getSeconds() / 3600;
        const solution = altDay ? this._minPhEquationSolutionForAltDay : this._minPhEquationSolutionForNormDay;

        return calcMinPh({
            config: this._configService.config.phController,
            solution,
            hour,
            altDay
        });
    }
}
