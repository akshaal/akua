import { injectable } from "inversify";

export interface Temperature {
    readonly value: number | null;
    readonly valueSamples: number;
    readonly disconnects: number;
    readonly crcErrors: number;
}

@injectable()
export default abstract class TemperatureSensorService {
    abstract aquariumTemperature: Temperature | null;
    abstract caseTemperature: Temperature | null;
}