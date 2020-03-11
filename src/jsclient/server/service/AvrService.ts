import { injectable } from "inversify";
import type { Observable } from "rxjs";

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

export interface AvrLightState {
    readonly dayLightOn: boolean;
    readonly dayLightForced: boolean;
    readonly nightLightOn: boolean;
    readonly nightLightForced: boolean;
    readonly lightForcesSinceProtectionStatReset: number;
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
    readonly mainLoopIterationsInLastDecisecond: number;
    readonly uptimeSeconds: number;
    readonly clockDriftSeconds: number;
    readonly clockCorrectionsSinceProtectionStatReset: number;
    readonly clockSecondsSinceMidnight: number;
    readonly debugOverflows: number;
    readonly usbRxOverflows: number;
    readonly co2Sensor: AvrCo2SensorState;
    readonly aquariumTemperatureSensor: AvrTemperatureSensorState;
    readonly caseTemperatureSensor: AvrTemperatureSensorState;
    readonly light: AvrLightState;
}

export enum LightForceMode {
    NotForced = 0,
    Day = 1,
    Night = 2
};

@injectable()
export default abstract class AvrService {
    readonly abstract avrState$: Observable<AvrState>;

    abstract getServiceState(): AvrServiceState;

    abstract forceLight(mode: LightForceMode): void;
}