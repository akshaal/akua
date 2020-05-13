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
    readonly phToTurnOff: number;
    readonly phToTurnOn: number;
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
    readonly co2Display: ValueDisplayConfig;
    readonly database: DatabaseConfig;
    readonly phClosingPrediction: PhClosingPredictionConfig;
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
    phToTurnOff: 6.8,
    phToTurnOn: 6.9,
};

const phClosingPrediction: PhClosingPredictionConfig = {
    trainDatasetPercentage: 0.95,
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
    phDisplay,
    co2Display,
    phController,
    phClosingPrediction,
    database
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
