import { injectable } from "inversify";
import type { AvrTemperatureSensorState } from "./AvrService";

export interface Temperature {
    readonly value: number | null;
    readonly valueSamples: number;
    readonly lastSensorState: AvrTemperatureSensorState | null;
}

@injectable()
export default abstract class TemperatureSensorService {
    abstract aquariumTemperature: Temperature | null;
    abstract caseTemperature: Temperature | null;
}