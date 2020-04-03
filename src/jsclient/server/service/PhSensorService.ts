import { injectable } from "inversify";
import { AvrPhState } from "./AvrService";
import { Observable } from "rxjs";

export interface Ph {
    readonly voltage5s: number | null;
    readonly voltage5sSamples: number;
    readonly value60s: number | null;
    readonly value60sSamples: number;
    readonly value600s: number | null;
    readonly value600sSamples: number;
    readonly lastSensorState: AvrPhState | null;
}

@injectable()
export default abstract class PhSensorService {
    readonly abstract ph: Ph | null;
    readonly abstract ph$: Observable<Ph | null>;
}
