import { injectable } from "inversify";
import Co2SensorService, { Co2 } from "server/service/Co2SensorService";
import AvrService, { AvrCo2SensorState } from "server/service/AvrService";
import { AveragingWindow } from "./AveragingWindow";

const CO2_WINDOW_SPAN_SECONDS = 60 * 5; // 3 minutes
const CO2_SAMPLE_FREQUENCY = 1; // How many measurements per second our AVR performs

// ==========================================================================================

class SensorProcessor {
    private _prevState?: AvrCo2SensorState;
    private _avgWindow = new AveragingWindow(CO2_WINDOW_SPAN_SECONDS, CO2_SAMPLE_FREQUENCY);

    onNewAvrState(newState: AvrCo2SensorState) {
        if (this._prevState && this._prevState.updateId != newState.updateId) {
            if (newState.updatedSecondsAgo < 200 && newState.concentration > 0 && newState.concentration < 5000) {
                this._avgWindow.add(newState.concentration);
            }
        }

        this._prevState = newState;
    }

    get(): Co2 | null {
        if (!this._prevState) {
            return null;
        }

        return {
            value: this._avgWindow.get(),
            valueSamples: this._avgWindow.getCount(),
            crcErrors: this._prevState.crcErrors,
            rxOverflows: this._prevState.rxOverflows,
            abcSetups: this._prevState.abcSetups,
            temperature: this._prevState.temperature,
            s: this._prevState.s,
            u: this._prevState.u
        };
    }
}

@injectable()
export default class Co2SensorServiceImpl extends Co2SensorService {
    private _co2SensorProcessor = new SensorProcessor();

    constructor(_avrService: AvrService) {
        super();
        _avrService.avrState$.subscribe(avrState => {
            this._co2SensorProcessor.onNewAvrState(avrState.co2Sensor);
        });
    }

    get co2(): Co2 | null {
        return this._co2SensorProcessor.get();
    }
}