import { injectable } from "inversify";
import type { AvrTemperatureSensorState } from "./AvrService";
import { Observable } from "rxjs";

export interface Temperature {
    readonly value: number | null;
    readonly valueSamples: number;
    readonly lastSensorState: AvrTemperatureSensorState | null;
}

@injectable()
export default abstract class TemperatureSensorService {
    readonly abstract aquariumTemperature: Temperature | null;
    readonly abstract aquariumTemperature$: Observable<Temperature | null>;

    readonly abstract caseTemperature: Temperature | null;
    readonly abstract caseTemperature$: Observable<Temperature | null>;
}