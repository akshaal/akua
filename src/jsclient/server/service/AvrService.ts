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
    readonly outgoingMessages: number;
    readonly lastAvrState?: AvrState;
}

export interface AvrTemperatureSensorState {
    readonly updateId: number;
    readonly crcErrors: number;
    readonly disconnects: number;
    readonly temperature: number;
    readonly updatedSecondsAgo: number;
}

export interface AvrCo2SensorState {
    readonly updateId: number;
    readonly crcErrors: number;
    readonly abcSetups: number;
    readonly concentration: number;
    readonly rawConcentration: number;
    readonly clampedConcentration: number;
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

export interface AvrControlState {
    readonly nightLightSwitchOn: boolean;
    readonly dayLightSwitchOn: boolean;
}

@injectable()
export default abstract class AvrService {
    readonly abstract avrState$: Observable<AvrState>;
    readonly abstract requestedControlState$: Observable<AvrControlState>;

    abstract getServiceState(): AvrServiceState;

    abstract requestControlState(controlState: AvrControlState): void;
}