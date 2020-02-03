import { injectable } from "inversify";

export interface Co2 {
    readonly value: number | null;
    readonly valueSamples: number;
    readonly rxOverflows: number;
    readonly crcErrors: number;
    readonly abcSetups: number;
    readonly temperature: number;
    readonly s: number;
    readonly u: number;
}

@injectable()
export default abstract class Co2SensorService {
    abstract co2: Co2 | null;
}