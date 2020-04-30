import { injectable, postConstruct, optional, inject } from "inversify";
import { Worker } from "worker_threads";
import { Subscriptions } from "server/misc/Subscriptions";
import { MessageFromPhPredictionWorker, MinPhPredictionRequest, createCo2ClosingState, Co2ClosingStateOrigin } from "./PhPrediction";
import logger from "server/logger";
import PhPredictionService, { MinClosingPhPrediction } from "server/service/PhPredictionService";
import { Subject, SchedulerLike, timer } from "rxjs";
import { getElapsedSecondsSince } from "server/misc/get-elapsed-seconds-since";
import PhSensorService from "server/service/PhSensorService";
import AvrService from "server/service/AvrService";
import TimeService from "server/service/TimeService";

@injectable()
export default class PhPredictionServiceImpl extends PhPredictionService {
    readonly minClosingPhPrediction$ = new Subject<MinClosingPhPrediction>();;

    // Key is rounded time (Math.round). Value is a PH at the given time.
    // These maps are cleanup up at midnight.
    private _ph600sMap: { [t: number]: number } = {};
    private _ph60sMap: { [t: number]: number } = {};

    // Last known state of CO2-valve switch
    private _co2ValveOpen: boolean = false;

    private readonly _subs = new Subscriptions();
    private _worker?: Worker;
    private _lastMinPhPrediction?: number;

    constructor(
        private readonly _timeService: TimeService,
        private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService,
        @optional() @inject("scheduler") private readonly _scheduler: SchedulerLike
    ) {
        super();
    }

    @postConstruct()
    _init(): void {
        // Create worker thread that will actually perform predictions
        this._worker = new Worker("./dist/server/service_impl/PhPredictionWorkerThread.js");

        // React on messages from worker thread
        this._worker.on('message', (message: MessageFromPhPredictionWorker) => {
            if (message.type === 'min-ph-prediction-response') {
                this.minClosingPhPrediction$.next({
                    predictedMinPh: message.minPhPrediction,
                    secondsUsedOnPrediction: getElapsedSecondsSince(message.requestTimestamp),
                    valveIsAlreadyClosed: false
                });
            } else {
                logger.error("PhPredictService: Unknown message type", { message });
            }
        });

        // We need PH values
        this._subs.add(
            this._phSensorService.ph$.subscribe(ph => {
                const t = this._timeService.nowRoundedSeconds();
                const ph600s = ph?.value600s;
                const ph60s = ph?.value60s;

                if (ph600s) {
                    this._ph600sMap[t] = ph600s;
                }

                if (ph60s) {
                    this._ph60sMap[t] = ph60s;
                }
            })
        );

        // We need CO2 valve switch state
        this._subs.add(
            this._avrService.avrState$.subscribe(avrState => {
                this._co2ValveOpen = avrState.co2ValveOpen;
            })
        );

        // Perform prediction every 2 seconds.
        this._subs.add(
            timer(0, 2000, this._scheduler).subscribe(() => {
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
    }

    // Called at night to cleanup maps so we don't run out of memory
    _cleanupHistoryMaps(): void {
        // It's not a problem to just remove everything from the map
        // because we need the history only at CO2-day time and only for last 15 or so minutes.
        this._ph600sMap = {};
        this._ph60sMap = {};
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
        if (!this._co2ValveOpen) {
            if (this._lastMinPhPrediction) {
                this.minClosingPhPrediction$.next({
                    predictedMinPh: this._lastMinPhPrediction,
                    secondsUsedOnPrediction: 0,
                    valveIsAlreadyClosed: true
                });
            }
            return;
        }

        // Create CO2 Closing state which will be used by working thread to do prediction
        // TODO: Remember this request somewhere as lastRequest so it can be used for training
        const co2ClosingState =
            createCo2ClosingState({
                tClose: this._timeService.nowRoundedSeconds(),
                minPh600: 7, // doesn't matter
                origin: Co2ClosingStateOrigin.ThisInstance,
                getPh600: (t: number) => this._ph600sMap[t],
                getPh60: (t: number) => this._ph60sMap[t],
            });

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
