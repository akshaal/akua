import { injectable } from "inversify";

export interface AvrServiceState {
    readonly serialPortErrors: number;
    readonly serialPortOpenAttempts: number;
    readonly serialPortIsOpen: 0 | 1;
    readonly protocolVersionMismatch: 0 | 1;
    readonly protocolCrcErrors: number;
    readonly protocolDebugMessages: number;
}

@injectable()
export default abstract class AvrService {
    abstract getState(): AvrServiceState;
}