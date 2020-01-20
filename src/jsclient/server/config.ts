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

export interface Config {
    readonly version: string;
    readonly isProd: boolean;
    readonly isDev: boolean;
    readonly bindOptions: ProtoHostPort;
    readonly metrics: MetricsConfig;
    readonly nextion: NextionConfig;
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
    port: "/dev/ttyAMA0"
}

export const config: Config = {
    isDev,
    version,
    isProd: !isDev,
    bindOptions,
    metrics,
    nextion
};

// ================================================================================================

export default config;
