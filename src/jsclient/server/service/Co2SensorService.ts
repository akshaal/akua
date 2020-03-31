import { injectable } from "inversify";
import type { AvrCo2SensorState } from "./AvrService";

export interface Co2 {
    readonly value: number | null;
    readonly valueSamples: number;
    readonly lastSensorState: AvrCo2SensorState | null;
    readonly sensorBoot: boolean;
    readonly sensorUptimeSeconds: number;
    readonly warmup: boolean;
}

@injectable()
export default abstract class Co2SensorService {
    readonly abstract co2: Co2 | null;
}