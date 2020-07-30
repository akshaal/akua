import process from "process";

// ================================================================================================

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
    readonly isProd: boolean;
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

// ================================================================================================

const isDev = (process.env.NODE_ENV !== "production") || (process.env.AKUA_FORCE_DEV == "true");

// ================================================================================================

export function asUrl(options: ProtoHostPort): string {
    if ((options.port == 443 && options.proto == "https") || (options.port == 80 && options.proto == "http")) {
        return `${options.proto}://${options.host}`;
    }

    return `${options.proto}://${options.host}:${options.port}`;
}

// ================================================================================================

const instanceName = process.env.AKUA_INSTANCE || "unknown";

if (instanceName !== "aqua1" && instanceName !== "aqua2") {
    console.log("::ERROR:: UNKNOWN INSTANCE: " + instanceName);
}

function aquaX<T>(configs: { aqua1: T, aqua2: T }): T {
    if (instanceName === "aqua1") {
        return configs.aqua1;
    } else {
        return configs.aqua2;
    }
}

const version = process.env.AKUA_VERSION || "unknown";
const bindOptions: ProtoHostPort = {
    port: 3000,
    host: "0.0.0.0", // Must bind to 0.0.0.0. Binding to 127.0.0.1 makes port-forwarding impossible in docker
    proto: "http"
};

const metrics: MetricsConfig = {
    eventLoopMonitoringResolutionMs: 50,
    autocleanupNotActiveForSeconds: 60 * 60 * 24 * 7, // one week
    autocleanupIntervalSeconds: 60 * 60,
    sessionsCountingIntervalSeconds: 15,
};

const nextion: NextionConfig = {
    port: process.env.AKUA_NEXTION_PORT || "/dev/ttyAMA0"
};

const avr: AvrConfig = {
    port: process.env.AKUA_PORT || "/dev/ttyUSB0"
};

const aquaTemperatureDisplay: ValueDisplayConfig = {
    lowComfort: 23.6,
    highComfort: 24.5,
    lowTolerable: 22,
    highTolerable: 27,
    comfortRgbPcts: [100, 100, 100],
    lowRgbPcts: [0, 0, 100],
    highRgbPcts: [100, 0, 0],
};

const caseTemperatureDisplay: ValueDisplayConfig = {
    lowComfort: 17.5,
    highComfort: 40,
    lowTolerable: 10,
    highTolerable: 50,
    comfortRgbPcts: [100, 100, 100],
    lowRgbPcts: [0, 0, 100],
    highRgbPcts: [100, 0, 0],
};

const phDisplay: ValueDisplayConfig = {
    lowComfort: 6.8,
    highComfort: 7.2,
    lowTolerable: 6.5,
    highTolerable: 8,
    comfortRgbPcts: [100, 100, 100],
    lowRgbPcts: [100, 0, 0],
    highRgbPcts: [0, 0, 100],
};

const co2Display: ValueDisplayConfig = {
    lowComfort: 15,
    highComfort: 22,
    lowTolerable: 1,
    highTolerable: 30,
    comfortRgbPcts: [100, 100, 100],
    lowRgbPcts: [100, 100, 0],
    highRgbPcts: [100, 0, 0],
};

const phController: PhControllerConfig = {
    phTurnOnOffMargin: 0.1,
    minSafePh600: 6.8,
    minSafePh60: 6.6,
    dayStartPh: 6.8,
    dayEndPh: 7.2,

    // These parameters must be in harmony with the values in the firmware!
    normDayPrepareHour: 8,
    normDayStartHour: 10,
    normDayEndHour: 22,
    altDayPrepareHour: 8,
    altDayStartHour: 10,
    altDayEndHour: 18,
};

const phClosingPrediction: PhClosingPredictionConfig = {
    trainDatasetPercentage: 0.90,
};

const phSensorCalibration: PhSensorCalibrationConfig = aquaX({
    // 2020.03.26: 4.01=3.08v,                6.86=2.575
    // 2020.05.17: 4.01=3.09559736896566v,    6.86=2.5856278083541535
    aqua1: {
        ph1: 4.01,
        v1: 3.09559736896566,
        ph2: 6.86,
        v2: 2.5856278083541535
    },

    // 2020.07.28: 4.01=3.0807776310782327,   6.86=2.616606056118044
    aqua2: {
        ph1: 4.01,
        v1: 3.0807776310782327,
        ph2: 6.86,
        v2: 2.616606056118044
    },
});

const aquaEnv: AquaEnvConfig = {
    kh: 4,
    alternativeDay: aquaX({ aqua1: false, aqua2: true })
};

const database: DatabaseConfig = {
    baseDirectory: isDev ? "../../temp/db" : "/var/akua/db",
    phClosingStateDbFileName: "ph-closing-state.db"
};

export const config: Config = {
    isDev,
    version,
    isProd: !isDev,
    instanceName,
    bindOptions,
    metrics,
    nextion,
    avr,
    caseTemperatureDisplay,
    aquaTemperatureDisplay,
    phSensorCalibration,
    phDisplay,
    co2Display,
    phController,
    phClosingPrediction,
    database,
    aquaEnv
};

// ================================================================================================

var errorsDetected = false;

if (config.avr.port === config.nextion.port) {
    console.log("::ERROR:: AVR and NEXTION ports configured to the same value: " + config.avr.port);
    errorsDetected = true;
}

if (errorsDetected) {
    console.log("F A T A L  E R R O R S  D E T E C T E D:  E X I S T I N G  N A O!");
    process.exit(2);
}

export default config;
