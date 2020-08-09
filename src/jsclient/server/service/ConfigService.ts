import { injectable } from "inversify";

export interface ProtoHostPort {
    readonly port: number;
    readonly host: string;
    readonly proto: string;
}

export interface MetricsConfig {
    // How often we measure eventloop performance
    readonly eventLoopMonitoringResolutionMs: number;
    readonly autocleanupNotActiveForSeconds: number;
    readonly autocleanupIntervalSeconds: number;
    readonly sessionsCountingIntervalSeconds: number;
}

export interface NextionConfig {
    readonly port: string;
}

export interface AvrConfig {
    readonly port: string;
}

export interface ValueDisplayConfig {
    readonly lowComfort: number;
    readonly highComfort: number;
    readonly lowTolerable: number;
    readonly highTolerable: number;
    readonly comfortRgbPcts: Readonly<[number, number, number]>;
    readonly lowRgbPcts: Readonly<[number, number, number]>;
    readonly highRgbPcts: Readonly<[number, number, number]>;
}

export interface PhControllerConfig {
    readonly minSafePh600: number,
    readonly minSafePh60: number,
    readonly phTurnOnOffMargin: number,
    readonly normDayPrepareHour: number,
    readonly normDayStartHour: number,
    readonly normDayEndHour: number,
    readonly altDayPrepareHour: number,
    readonly altDayStartHour: number,
    readonly altDayEndHour: number,
    readonly dayStartPh: number,
    readonly dayEndPh: number,
}

export interface PhSensorCalibrationConfig {
    /**
     * Calibration point 1: ph. Usually 4.01.
     */
    readonly ph1: number;

    /**
     * Voltage for calibration PH defined in ph1.
     */
    readonly v1: number;

    /**
     * Calibration point 2: ph. Usually 6.86.
     */
    readonly ph2: number;

    /**
     * Voltage for calibration PH defined in ph2.
     */
    readonly v2: number;
}

export interface PhClosingPredictionConfig {
    /**
     * Defines how we split dataset into training and test (validation).
     */
    readonly trainDatasetPercentage: number;
}

export interface DatabaseConfig {
    readonly baseDirectory: string;
    readonly phClosingStateDbFileName: string;
}

export interface AquaEnvConfig {
    readonly kh: number;

    /**
     * Depends on stuff in AVR firmware but intended to mean shorter day with
     * a nap time in the middle of the day to fight algae.
     */
    readonly alternativeDay: boolean;
}

export interface Config {
    readonly version: string;
    readonly isDev: boolean;
    readonly instanceName: string;
    readonly bindOptions: ProtoHostPort;
    readonly metrics: MetricsConfig;
    readonly nextion: NextionConfig;
    readonly avr: AvrConfig;
    readonly caseTemperatureDisplay: ValueDisplayConfig;
    readonly aquaTemperatureDisplay: ValueDisplayConfig;
    readonly phDisplay: ValueDisplayConfig;
    readonly phController: PhControllerConfig;
    readonly phSensorCalibration: PhSensorCalibrationConfig;
    readonly co2Display: ValueDisplayConfig;
    readonly database: DatabaseConfig;
    readonly phClosingPrediction: PhClosingPredictionConfig;
    readonly aquaEnv: AquaEnvConfig;
}

@injectable()
export default abstract class ConfigService {
    readonly abstract config: Config;
}
