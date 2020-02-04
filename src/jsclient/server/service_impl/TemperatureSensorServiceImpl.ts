import { injectable } from "inversify";
import TemperatureSensorService, { Temperature } from "server/service/TemperatureSensorService";
import AvrService, { AvrTemperatureSensorState } from "server/service/AvrService";
import { AveragingWindow } from "./AveragingWindow";

const TEMPERATURE_WINDOW_SPAN_SECONDS = 15;
const TEMPERATURE_SAMPLE_FREQUENCY = 1; // How many measurements per second our AVR performs

// ==========================================================================================

class SensorProcessor {
    private _prevState: AvrTemperatureSensorState | null = null;
    private _avgWindow = new AveragingWindow(TEMPERATURE_WINDOW_SPAN_SECONDS, TEMPERATURE_SAMPLE_FREQUENCY);

    onNewAvrState(newState: AvrTemperatureSensorState) {
        if (this._prevState && this._prevState.updateId != newState.updateId) {
            if (newState.updatedSecondsAgo < 200 && newState.temperature < 50 && newState.temperature > 0) {
                this._avgWindow.add(newState.temperature);
            }
        }

        this._prevState = newState;
    }

    get(): Temperature | null {
        if (!this._prevState) {
            return null;
        }
        
        return {
            value: this._avgWindow.get(),
            valueSamples: this._avgWindow.getCount(),
            lastSensorState: this._prevState
        };
    }
}

@injectable()
export default class TemperatureSensorServiceImpl extends TemperatureSensorService {
    private _aquariumSensorProcessor = new SensorProcessor();
    private _caseSensorProcessor = new SensorProcessor();

    constructor(_avrService: AvrService) {
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
}