import { injectable } from "inversify";
import { AvrPhState } from "./AvrService";

export interface Ph {
    readonly voltage5s: number | null;
    readonly voltage5sSamples: number;
    readonly value60s: number | null;
    readonly value60sSamples: number;
    readonly lastSensorState: AvrPhState | null;
}

@injectable()
export default abstract class PhSensorService {
    abstract ph: Ph | null;
}
