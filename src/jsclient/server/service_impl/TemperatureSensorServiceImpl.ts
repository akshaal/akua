import { injectable } from "inversify";
import TemperatureSensorService, { Temperature } from "server/service/TemperatureSensorService";
import AvrService, { AvrTemperatureSensorState } from "server/service/AvrService";
import { AveragingWindow } from "../misc/AveragingWindow";
import { Observable, BehaviorSubject } from "rxjs";
import TimeService from "server/service/TimeService";

const TEMPERATURE_WINDOW_SPAN_SECONDS = 15;
const TEMPERATURE_SAMPLE_FREQUENCY = 1; // How many measurements per second our AVR performs

// ==========================================================================================

class SensorProcessor {
    readonly values$ = new BehaviorSubject<Temperature | null>(null);

    private readonly _avgWindow = new AveragingWindow({
        windowSpanSeconds: TEMPERATURE_WINDOW_SPAN_SECONDS,
        sampleFrequency: TEMPERATURE_SAMPLE_FREQUENCY,
        timeService: this._timeService
    });

    constructor(private _timeService: TimeService) {}

    onNewAvrState(newState: AvrTemperatureSensorState) {
        const prevState = this.get()?.lastSensorState;

        if (!prevState || prevState.updateId != newState.updateId) {
            if (newState.updatedSecondsAgo < 200 && newState.temperature < 50 && newState.temperature > 0) {
                this._avgWindow.add(newState.temperature);

                this.values$.next({
                    value: this._avgWindow.get(),
                    valueSamples: this._avgWindow.getCount(),
                    lastSensorState: newState
                });
            }
        }
    }

    get(): Temperature | null {
        return this.values$.value;
    }
}

@injectable()
export default class TemperatureSensorServiceImpl extends TemperatureSensorService {
    private _aquariumSensorProcessor = new SensorProcessor(this._timeService);
    private _caseSensorProcessor = new SensorProcessor(this._timeService);

    constructor(_avrService: AvrService, private _timeService: TimeService) {
        super();
        _avrService.avrState$.subscribe(avrState => {
            this._aquariumSensorProcessor.onNewAvrState(avrState.aquariumTemperatureSensor);
            this._caseSensorProcessor.onNewAvrState(avrState.caseTemperatureSensor);
        });
    }

    get aquariumTemperature(): Temperature | null {
        return this._aquariumSensorProcessor.get();
    }

    get caseTemperature(): Temperature | null {
        return this._caseSensorProcessor.get();
    }

    get aquariumTemperature$(): Observable<Temperature | null> {
        return this._aquariumSensorProcessor.values$;
    }

    get caseTemperature$(): Observable<Temperature | null> {
        return this._caseSensorProcessor.values$;
    }
}