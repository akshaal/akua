import { injectable } from "inversify";
import PhSensorService, { Ph } from "server/service/PhSensorService";
import AvrService, { AvrPhState } from "server/service/AvrService";
import { AveragingWindow } from "./AveragingWindow";

const Ph_SAMPLE_FREQUENCY = 7; // How many measurements per second our AVR performs

function calcPhFromVoltage(voltage: number): number {
    return 14 - voltage / 5.0 * 14.0; // TODO: Solve equation based upon 3point calibration
}

// ==========================================================================================

class SensorProcessor {
    private _prevState: AvrPhState | null = null;
    private _avg5sWindow = new AveragingWindow(5, Ph_SAMPLE_FREQUENCY);
    private _avg60sWindow = new AveragingWindow(60, Ph_SAMPLE_FREQUENCY);

    onNewAvrState(newState: AvrPhState) {
        if (this._prevState) {
            if (newState.voltage < 5 && newState.voltage >= 0) {
                this._avg5sWindow.add(newState.voltage);
                this._avg60sWindow.add(newState.voltage);
            }
        }

        this._prevState = newState;
    }

    get(): Ph | null {
        if (!this._prevState) {
            return null;
        }
        
        const avg5s = this._avg5sWindow.get();
        const avg60s = this._avg60sWindow.get();

        return {
            voltage5s: avg5s ? Math.round(avg5s * 100) / 100.0 : avg5s,
            voltage5sSamples: this._avg5sWindow.getCount(),
            value60s: avg60s ? Math.round(calcPhFromVoltage(avg60s) * 1000) / 1000.0 : avg60s,
            value60sSamples: this._avg60sWindow.getCount(),
            lastSensorState: this._prevState
        };
    }
}

@injectable()
export default class PhSensorServiceImpl extends PhSensorService {
    private _phProcessor = new SensorProcessor();

    constructor(_avrService: AvrService) {
        super();
        _avrService.avrState$.subscribe(avrState => {
            this._phProcessor.onNewAvrState(avrState.ph);
        });
    }

    get ph(): Ph | null {
        return this._phProcessor.get();
    }
}