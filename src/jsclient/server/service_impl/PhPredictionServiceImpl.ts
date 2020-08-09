import { injectable, postConstruct, optional, inject } from "inversify";
import { Worker } from "worker_threads";
import { Subscriptions } from "server/misc/Subscriptions";
import { MessageFromPhPredictionWorker, MinPhPredictionRequest, createCo2ClosingState, Co2ClosingStateOrigin, Co2ClosingState } from "../service/PhPrediction";
import logger from "server/logger";
import PhPredictionService, { MinClosingPhPrediction, PhPredictionDatasetStats } from "server/service/PhPredictionService";
import { Subject, SchedulerLike, timer } from "rxjs";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";
import PhSensorService from "server/service/PhSensorService";
import AvrService from "server/service/AvrService";
import TimeService from "server/service/TimeService";
import { Timestamp } from "server/misc/Timestamp";
import { isPresent } from "server/misc/isPresent";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import _ from "lodash";
import TemperatureSensorService from "server/service/TemperatureSensorService";
import RandomNumberService from "server/service/RandomNumberService";
import ConfigService from "server/service/ConfigService";

/**
 * We assume we can have a bias toward adding items as validation ones.
 * This is a minimum difference between calculated number of validation
 * items and real number of validation items. If the difference
 * is below this number, then we don't promote validation to training.
 * 
 * Note that we never promote training to validation!
 */
const VALIDATION_TRAINING_SET_ALLOWED_DIFF = 20;

// TODO: More comments and tests!

@injectable()
export default class PhPredictionServiceImpl extends PhPredictionService {
    readonly minClosingPhPrediction$ = new Subject<MinClosingPhPrediction>();
    readonly phPredictionDatasetStats$ = new Subject<PhPredictionDatasetStats>();

    // Key is rounded time (Math.round). Value is a PH at the given time.
    // These maps are cleanup up at midnight.
    private _ph600sMap: { [t: number]: number } = {};
    private _ph60sMap: { [t: number]: number } = {};
    private _temperatureMap: { [t: number]: number } = {};
    private _co2ValveOpenMap: { [t: number]: boolean } = {};
    private _dayLightOnMap: { [t: number]: boolean } = {};

    // Last known state of CO2-valve switch
    private _co2ValveOpen: boolean = false;
    private _co2ValveOpenT?: Timestamp;

    private readonly _subs = new Subscriptions();
    private _worker?: Worker;
    private _lastMinPhPrediction?: number;

    private _lastCo2ClosingStateForDatabaseSaving?: Co2ClosingState | null;
    private _minPhAfterCloseForDatabaseSaving?: number;

    constructor(
        private readonly _timeService: TimeService,
        private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService,
        private readonly _databaseService: DatabaseService,
        private readonly _temperatureSensorService: TemperatureSensorService,
        private readonly _randomNumberService: RandomNumberService,
        private readonly _configService: ConfigService,
        @optional() @inject("scheduler") private readonly _scheduler: SchedulerLike
    ) {
        super();
    }

    @postConstruct()
    _init(): void {
        // Create worker thread that will actually perform predictions
        this._worker = new Worker("./dist/server/service_impl/PhPredictionWorkerThread.js", {
            workerData: this._configService.config
        });

        // React on messages from worker thread
        this._worker.on('message', (message: MessageFromPhPredictionWorker) => {
            if (message.type === 'min-ph-prediction-response') {
                const now = this._timeService.nowTimestamp();
                this.minClosingPhPrediction$.next({
                    predictedMinPh: message.minPhPrediction,
                    secondsUsedOnPrediction: getElapsedSecondsSince({ now, since: message.requestTimestamp }),
                    valveIsAlreadyClosed: false
                });
            } else {
                logger.error("PhPredictService: Unknown message type", { message });
            }
        });

        // We need temperature
        this._subs.add(
            this._temperatureSensorService.aquariumTemperature$.subscribe(tempObj => {
                const temp = tempObj?.value;
                if (temp) {
                    const tNow = this._timeService.nowRoundedSeconds();
                    this._temperatureMap[tNow] = temp;
                }
            })
        );

        // We need PH values
        this._subs.add(
            this._phSensorService.ph$.subscribe(ph => {
                const tNow = this._timeService.nowRoundedSeconds();
                const ph600s = ph?.value600s;
                const ph60s = ph?.value60s;

                if (ph600s) {
                    this._ph600sMap[tNow] = ph600s;
                }

                if (ph60s) {
                    this._ph60sMap[tNow] = ph60s;
                }

                if (this._lastCo2ClosingStateForDatabaseSaving && ph600s) {
                    if (!this._minPhAfterCloseForDatabaseSaving || this._minPhAfterCloseForDatabaseSaving > ph600s) {
                        this._minPhAfterCloseForDatabaseSaving = ph600s
                    }
                }
            })
        );

        // We need CO2 valve switch state and also light state
        // We also use this one to save predictions and stuff
        // TODO: Simplify and move to some other place, tests....
        this._subs.add(
            this._avrService.avrState$.subscribe(avrState => {
                const tNow = this._timeService.nowRoundedSeconds();
                this._co2ValveOpenMap[tNow] = avrState.co2ValveOpen;
                this._dayLightOnMap[tNow] = avrState.light.dayLightOn;

                // Do stuff based upon co2 valve state
                if (this._co2ValveOpen && !avrState.co2ValveOpen) {
                    // Going from open to closed
                    this._co2ValveOpenT = undefined;

                    // Prevent using stale stuff
                    if (this._lastCo2ClosingStateForDatabaseSaving) {
                        const createdSecondsAgo = tNow - this._lastCo2ClosingStateForDatabaseSaving.closeTime;
                        if (createdSecondsAgo > 10) {
                            logger.error("Stale lastCo2ClosingStateForDatabaseSaving!", { state: this._lastCo2ClosingStateForDatabaseSaving });
                            this._lastCo2ClosingStateForDatabaseSaving = undefined;
                        }
                    }
                } else if (!this._co2ValveOpen && avrState.co2ValveOpen) {
                    // Going from closed to open. Remember this moment
                    this._co2ValveOpenT = this._timeService.nowTimestamp();

                    this._addDatasetItemIfValid(
                        this._lastCo2ClosingStateForDatabaseSaving,
                        this._minPhAfterCloseForDatabaseSaving
                    );

                    this._minPhAfterCloseForDatabaseSaving = undefined;
                    this._lastCo2ClosingStateForDatabaseSaving = undefined;
                }

                this._co2ValveOpen = avrState.co2ValveOpen;
            })
        );

        // Perform prediction every second.
        this._subs.add(
            timer(0, 1000, this._scheduler).subscribe(() => {
                this._requestPrediction();
            })
        );

        // Perform cleanup of used map at night (we check it every 10 minutes).
        this._subs.add(
            timer(0, 600_000, this._scheduler).subscribe(() => {
                const date = new Date(this._timeService.nowRoundedSeconds() * 1000.0);
                const hours = date.getHours();
                if (hours > 0 && hours < 3) {
                    this._cleanupHistoryMaps();
                }
            })
        );

        // Publish dataset statistics.
        this._subs.add(
            timer(0, 5_000, this._scheduler).subscribe(() => {
                this._publishDatasetStats();
            })
        );

        // Perform maintenance at start / initialization
        this._maintainDataset();
    }

    private async _publishDatasetStats(): Promise<void> {
        const phClosingStateValidationDatasetSize = await this._databaseService.countCo2ClosingStates(Co2ClosingStateType.VALIDATION);
        const phClosingStateTrainingDatasetSize = await this._databaseService.countCo2ClosingStates(Co2ClosingStateType.TRAINING);

        this.phPredictionDatasetStats$.next({
            phClosingStateValidationDatasetSize,
            phClosingStateTrainingDatasetSize
        });
    }

    /**
     * Adds new new validation or training dataset based upon results of observations.
     */
    private async _addDatasetItemIfValid(
        closingState?: Co2ClosingState | null,
        minPhAfterClose?: number
    ): Promise<void> {
        const tNow = this._timeService.nowRoundedSeconds();

        // Save co2 closing state and its outcome in the database
        if (closingState && minPhAfterClose) {
            const createdSecondsAgo = tNow - closingState.closeTime;

            // TODO: Move to config
            if (createdSecondsAgo < (7 * 60 * 60)) {
                const minPh600OffsetAfterClose = minPhAfterClose - closingState.ph600AtClose;
                const isValidation = this._randomNumberService.next() > this._configService.config.phClosingPrediction.trainDatasetPercentage;

                await this._databaseService.insertCo2ClosingState(
                    { ...closingState, minPh600OffsetAfterClose },
                    { isValidation }
                );

                // Make sure data is split into test data and validation.
                await this._maintainDataset();
            }
        }
    }

    /**
     * Maintain database. Make sure we do split data into training and validation/testing dataset.
     * This will make it possible to have a stable way to compare NN-performance / NN-algorithms / etc.
     */
    private async _maintainDataset(): Promise<void> {
        const totalSize = await this._databaseService.countCo2ClosingStates(Co2ClosingStateType.ANY);
        const currentValidationSize = await this._databaseService.countCo2ClosingStates(Co2ClosingStateType.VALIDATION);
        const expectedValidationSize = Math.floor(totalSize * (1 - this._configService.config.phClosingPrediction.trainDatasetPercentage));
        const numberOfStatesToConvertToTraining = currentValidationSize - expectedValidationSize;

        // Note that we never promote training to validation! Only other way around!

        if ((numberOfStatesToConvertToTraining - VALIDATION_TRAINING_SET_ALLOWED_DIFF) > 0) {
            logger.warn(`PhPredictService: Force-promoting ${numberOfStatesToConvertToTraining} validation items to training!`);

            const validationStateTimes = await this._databaseService.findCo2ClosingTimes(Co2ClosingStateType.VALIDATION);
            const statesToConvertToTraining = _.take(_.shuffle(validationStateTimes), numberOfStatesToConvertToTraining);
            await this._databaseService.markCo2ClosingStatesAsTraining(statesToConvertToTraining);
        }
    }

    // Called at night to cleanup maps so we don't run out of memory
    private _cleanupHistoryMaps(): void {
        // It's not a problem to just remove everything from the map
        // because we need the history only at CO2-day time and only for last 15 or so minutes.
        this._ph600sMap = {};
        this._ph60sMap = {};
        this._temperatureMap = {};
        this._dayLightOnMap = {};
        this._co2ValveOpenMap = {};
    }

    // Used in unit testing
    _destroy(): void {
        this._subs.unsubscribeAll();

        if (this._worker) {
            this._worker.terminate();
            this._worker = undefined;
        }
    }

    // Called by timer to request prediction from the working thread
    private _requestPrediction(): void {
        // Just emit previous value if CO2 Valve is closed
        if (!this._co2ValveOpen || !isPresent(this._co2ValveOpenT)) {
            if (this._lastMinPhPrediction) {
                this.minClosingPhPrediction$.next({
                    predictedMinPh: this._lastMinPhPrediction,
                    secondsUsedOnPrediction: 0,
                    valveIsAlreadyClosed: true
                });
            }
            return;
        }

        // TODO: Move to some common place
        function getNearest<T>(map: { [t: number]: T }, t: number, lookBack: number): T | undefined {
            const v = map[t];
            if (isPresent(v)) {
                return v;
            }

            if (lookBack > 0) {
                return getNearest(map, t - 1, lookBack - 1);
            }

            return v;
        }

        // Create CO2 Closing state which will be used by working thread to do prediction
        const tClose = this._timeService.nowRoundedSeconds();
        const now = this._timeService.nowTimestamp();
        const co2ClosingState =
            createCo2ClosingState({
                closeTime: tClose,
                openedSecondsAgo: getElapsedSecondsSince({ now, since: this._co2ValveOpenT }),
                minPh600: 7, // doesn't matter
                origin: Co2ClosingStateOrigin.ThisInstance,
                getPh600: (t: number) => getNearest(this._ph600sMap, t, 4),
                getPh60: (t: number) => getNearest(this._ph60sMap, t, 4),
                getTemperature: (t: number) => getNearest(this._temperatureMap, t, 4),
                isDayLightOn: (t: number) => getNearest(this._dayLightOnMap, t, 4),
                isCo2ValveOpen: (t: number) => t == tClose ? false : getNearest(this._co2ValveOpenMap, t, 4),
            });

        this._lastCo2ClosingStateForDatabaseSaving = co2ClosingState;
        this._minPhAfterCloseForDatabaseSaving = this._ph600sMap[tClose];

        // Just emit previous prediction if we don't have enough information yet
        if (!co2ClosingState) {
            return;
        }

        // Prepare request and perform it
        const request: MinPhPredictionRequest = {
            type: 'min-ph-prediction-request',
            co2ClosingState
        };

        if (this._worker) {
            this._worker.postMessage(request);
        }
    }
}
