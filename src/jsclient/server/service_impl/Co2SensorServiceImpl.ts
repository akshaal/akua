import { injectable } from "inversify";
import Co2SensorService, { Co2 } from "server/service/Co2SensorService";
import AvrService, { AvrCo2SensorState } from "server/service/AvrService";
import { AveragingWindow } from "./AveragingWindow";
import { newTimestamp } from "./new-timestamp";
import { getElapsedSecondsSince } from "./get-elapsed-seconds-since";
import type { Timestamp } from "./Timestamp";

const CO2_WINDOW_SPAN_SECONDS = 60 * 3; // 3 minutes
const CO2_SAMPLE_FREQUENCY = 1; // How many measurements per second our AVR performs
const WARMUP_SECONDS = 60 * 10;

// ==========================================================================================

class SensorProcessor {
    private _prevState: AvrCo2SensorState | null = null;
    private _avgWindow = new AveragingWindow(CO2_WINDOW_SPAN_SECONDS, CO2_SAMPLE_FREQUENCY);
    private _sensorBoot = true;
    private _sensorBootTimestamp: Timestamp = newTimestamp();
    private _warmup = true;

    onNewAvrState(newState: AvrCo2SensorState, avrUptimeSeconds: number) {
        if (this._prevState && this._prevState.updateId != newState.updateId) {
            this._sensorBoot = newState.clampedConcentration == 410 && (newState.concentration > 420 || newState.concentration <= 400);

            if (this._sensorBoot) {
                this._sensorBootTimestamp = newTimestamp();
            }

            const avrWarmup = avrUptimeSeconds < WARMUP_SECONDS;
            const sensorWarmup = getElapsedSecondsSince(this._sensorBootTimestamp) < WARMUP_SECONDS;
            this._warmup = avrWarmup || sensorWarmup;

            if (!this._warmup && newState.updatedSecondsAgo < 200 && newState.concentration > 0 && newState.concentration < 5000 && newState.temperature > 10 && newState.temperature < 40) {
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
            lastSensorState: this._prevState,
            sensorBoot: this._sensorBoot,
            sensorUptimeSeconds: getElapsedSecondsSince(this._sensorBootTimestamp),
            warmup: this._warmup
        };
    }
}

@injectable()
export default class Co2SensorServiceImpl extends Co2SensorService {
    private _co2SensorProcessor = new SensorProcessor();

    constructor(_avrService: AvrService) {
        super();
        _avrService.avrState$.subscribe(avrState => {
            this._co2SensorProcessor.onNewAvrState(avrState.co2Sensor, avrState.uptimeSeconds);
        });
    }

    get co2(): Co2 | null {
        return this._co2SensorProcessor.get();
    }
}