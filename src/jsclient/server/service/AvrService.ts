import { injectable } from "inversify";
import { Observable } from "rxjs";

export interface AvrServiceState {
    readonly serialPortErrors: number;
    readonly serialPortOpenAttempts: number;
    readonly serialPortIsOpen: 0 | 1;
    readonly protocolVersionMismatch: 0 | 1;
    readonly protocolCrcErrors: number;
    readonly protocolDebugMessages: number;
    readonly incomingMessages: number;
}

export interface AvrTemperatureSensorState {
    readonly crcErrors: number;
    readonly disconnects: number;
    readonly temperature: number;
    readonly updatedSecondsAgo: number;
}

export interface AvrCo2SensorState {
    readonly crcErrors: number;
    readonly abcSetups: number;
    readonly concentration: number;
    readonly temperature: number;
    readonly s: number;
    readonly u: number;
    readonly updatedSecondsAgo: number;
    readonly rxOverflows: number;
}

export interface AvrState {
    readonly uptimeSeconds: number;
    readonly debugOverflows: number;
    readonly usbRxOverflows: number;
    readonly co2Sensor: AvrCo2SensorState;
    readonly aquariumTemperatureSensor: AvrTemperatureSensorState;
    readonly caseTemperatureSensor: AvrTemperatureSensorState;
}

@injectable()
export default abstract class AvrService {
    readonly abstract avrState$: Observable<AvrState>;

    abstract getState(): AvrServiceState;
}