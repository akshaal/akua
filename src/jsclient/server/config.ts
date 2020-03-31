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

export interface Config {
    readonly version: string;
    readonly isProd: boolean;
    readonly isDev: boolean;
    readonly bindOptions: ProtoHostPort;
    readonly metrics: MetricsConfig;
    readonly nextion: NextionConfig;
    readonly avr: AvrConfig;
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
}

const nextion: NextionConfig = {
    port: process.env.AKUA_NEXTION_PORT || "/dev/ttyAMA0"
}

const avr: AvrConfig = {
    port: process.env.AKUA_PORT || "/dev/ttyUSB0"
}

export const config: Config = {
    isDev,
    version,
    isProd: !isDev,
    bindOptions,
    metrics,
    nextion,
    avr
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
