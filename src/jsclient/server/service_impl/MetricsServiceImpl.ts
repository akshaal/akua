import { injectable } from "inversify";
import { Gauge, register, Summary, Counter } from 'prom-client';
import perfHooks from 'perf_hooks';
import config from "server/config";
import { getInfoCount, getErrorCount, getWarningCount } from "server/logger";
import MetricsService from "server/service/MetricsService";
import AvrService from "server/service/AvrService";
import TemperatureSensorService, { Temperature } from "server/service/TemperatureSensorService";
import Co2SensorService from "server/service/Co2SensorService";

// Use constants for labels to avoid typos and to be consistent about names.
const L_GC_TYPE = 'gc_type';
const L_VERSION = 'version';
const L_TARGET = "target";
const L_LEVEL = "level";

// ==========================================================================================

class SettableSimpleCounter extends Counter {
    constructor(config: { name: string, help: string }) {
        super(config);
    }

    set(value: number) {
        this.reset();
        this.inc(value);
    }

    setOrRemove(value?: number) {
        if (typeof value === "number") {
            this.set(value);
        } else {
            this.remove();
        }
    }
}

// ==========================================================================================

const eventLoopSummary = new Summary({
    name: 'akua_nodejs_event_loop_summary',
    help: 'Summary of event loop delays.',
    maxAgeSeconds: 600,
    ageBuckets: 5,
    percentiles: [0.1, 0.25, 0.5, 0.75, 0.9, 0.99, 0.999]
});

function reportEventloopLag(start: [number, number]) {
    const delta = process.hrtime(start);
    const nanoseconds = delta[0] * 1e9 + delta[1];
    const seconds = nanoseconds / 1e9;

    eventLoopSummary.observe(seconds);

    // eslint-disable-next-line @typescript-eslint/no-use-before-define
    scheduleEventloopReporting();
}

function scheduleEventloopReporting(): void {
    setTimeout(() => {
        const start = process.hrtime();
        setImmediate(reportEventloopLag, start);
    }, config.metrics.eventLoopMonitoringResolutionMs);
}

scheduleEventloopReporting();

// ==========================================================================================

const gcSummary = new Summary({
    name: 'akua_nodejs_gc_duration_summary',
    help: 'Summary of garbage collections. gc_type label is one of major, minor, incremental or weakcb.',
    labelNames: [L_GC_TYPE],
    maxAgeSeconds: 600,
    ageBuckets: 5,
    percentiles: [0.5, 0.75, 0.9, 0.99]
});

function gcKindToString(gcKind: number | undefined) {
    let gcKindName = '';

    switch (gcKind) {
        case perfHooks.constants.NODE_PERFORMANCE_GC_MAJOR:
            gcKindName = 'major';
            break;
        case perfHooks.constants.NODE_PERFORMANCE_GC_MINOR:
            gcKindName = 'minor';
            break;
        case perfHooks.constants.NODE_PERFORMANCE_GC_INCREMENTAL:
            gcKindName = 'incremental';
            break;
        case perfHooks.constants.NODE_PERFORMANCE_GC_WEAKCB:
            gcKindName = 'weakcb';
            break;
        default:
            gcKindName = 'unknown';
            break;
    }

    return gcKindName;
}

const gcObserver = new perfHooks.PerformanceObserver(list => {
    const entry = list.getEntries()[0];

    // eslint-disable-next-line @typescript-eslint/camelcase
    const labels = { [L_GC_TYPE]: gcKindToString(entry.kind) };

    gcSummary.observe(labels, entry.duration / 1000);
});

gcObserver.observe({ entryTypes: ['gc'], buffered: false });

// ==========================================================================================

const cpuUserUsageGauge = new Gauge({
    name: 'akua_process_cpu_user_pct',
    help: 'User CPU time by node process.'
});

const cpuSystemUsageGauge = new Gauge({
    name: 'akua_process_cpu_system_pct',
    help: 'System CPU time by node process.'
});

let lastCpuUsage = process.cpuUsage();
let lastCpuUsageTimestamp = process.hrtime();

function updateCpuUsageMetrics() {
    const delta = process.hrtime(lastCpuUsageTimestamp);
    const elapsedNanoseconds = delta[0] * 1e9 + delta[1];
    const elapsedSeconds = elapsedNanoseconds / 1e9;

    const cpuUsage = process.cpuUsage();

    const userUsageMicros = cpuUsage.user - lastCpuUsage.user;
    const systemUsageMicros = cpuUsage.system - lastCpuUsage.system;

    lastCpuUsage = cpuUsage;

    cpuUserUsageGauge.set(100 * userUsageMicros / 1e6 / elapsedSeconds);
    cpuSystemUsageGauge.set(100 * systemUsageMicros / 1e6 / elapsedSeconds);

    lastCpuUsageTimestamp = process.hrtime();
}

// ==========================================================================================

const processStartTimeGauge = new Gauge({
    name: 'akua_process_start_time_seconds',
    help: 'Start time of the process since unix epoch in seconds.',
    aggregator: 'omit'
});

processStartTimeGauge.set(Math.round(Date.now() / 1000 - process.uptime()));

// ==========================================================================================

const residentMemGauge = new Gauge({
    name: 'akua_process_resident_memory_bytes',
    help: 'Resident memory size in bytes.',
});

const heapTotalMemGauge = new Gauge({
    name: 'akua_process_heap_total_bytes',
    help: 'Total heap memory size in bytes.',
});

const heapUsedMemGauge = new Gauge({
    name: 'akua_process_heap_used_bytes',
    help: 'Used heap memory size in bytes.',
});

const externalMemGauge = new Gauge({
    name: 'akua_process_external_memory_bytes',
    help: 'Used heap memory size in bytes.',
});

function updateMemUsageMetrics() {
    const memUsage = process.memoryUsage();

    residentMemGauge.set(memUsage.rss);
    heapTotalMemGauge.set(memUsage.heapTotal);
    heapUsedMemGauge.set(memUsage.heapUsed);
    externalMemGauge.set(memUsage.external);
}

// ==========================================================================================

const versionGauge = new Gauge({
    name: 'akua_startup_info',
    help: 'Info on startup.',
    labelNames: [L_VERSION]
});

// For Grafana... it needs value as time in milliseconds to enable de-duplication of annotation.
const startupTimeMs = new Date().getTime();
versionGauge.set({ version: config.version }, startupTimeMs, new Date);

// ==========================================================================================

const simpleMeasurementSummary = new Summary({
    name: 'akua_simple_measure_summary',
    help: 'Simple measurements.',
    maxAgeSeconds: 600,
    ageBuckets: 5,
    percentiles: [0.1, 0.25, 0.5, 0.75, 0.9, 0.99, 0.999],
    labelNames: [L_TARGET]
});

// ==========================================================================================

const loggingCountGauge = new Counter({
    name: 'akua_logging_count',
    help: 'Number of log messages.',
    labelNames: [L_LEVEL]
});

function updateLoggingMetrics() {
    loggingCountGauge.reset();
    loggingCountGauge.inc({ [L_LEVEL]: 'info' }, getInfoCount());
    loggingCountGauge.inc({ [L_LEVEL]: 'error' }, getErrorCount());
    loggingCountGauge.inc({ [L_LEVEL]: 'warning' }, getWarningCount());
}

// ==========================================================================================

const avrUptimeSecondsGauge = new SettableSimpleCounter({
    name: 'akua_avr_uptime_seconds',
    help: 'Uptime seconds as returned by AVR (might be inaccurate as there is no RTC there).'
});

const avrDebugOverflowsGauge = new SettableSimpleCounter({
    name: 'akua_avr_debug_overflows',
    help: 'Number of times AVR was out of buffer space trying to send debug info to host.'
});

const avrUsbRxOverflowsGauge = new SettableSimpleCounter({
    name: 'akua_avr_usb_rx_overflows',
    help: 'Number of times AVR was out of buffer trying to receive data from USB.'
});

const avrSerialPortErrorCountGauge = new SettableSimpleCounter({
    name: 'akua_avr_serial_port_errors',
    help: 'Number of AVR serial port errors.'
});

const avrSerialPortOpenAttemptCountGauge = new SettableSimpleCounter({
    name: 'akua_avr_serial_port_open_attempts',
    help: 'Number of attempts to open AVR serial port.'
});

const avrProtocolCrcErrorCountGauge = new SettableSimpleCounter({
    name: 'akua_avr_protocol_crc_errors',
    help: 'Number of CRC errors (when decoding incoming message from AVR).'
});

const avrProtocolDebugMessageCountGauge = new SettableSimpleCounter({
    name: 'akua_avr_protocol_debug_messages',
    help: 'Number of messages received from AVR.'
});

const avrIncomingMessageCountGauge = new SettableSimpleCounter({
    name: 'akua_avr_incoming_messages',
    help: 'Total number of messages received from AVR.'
});

const avrSerialPortIsOpenGauge = new Gauge({
    name: 'akua_avr_port_is_open',
    help: '1 means that serial port is currently open, 0 means it is closed.'
});

const avrProtocolVersionMismatchGauge = new Gauge({
    name: 'akua_avr_protocol_version_mismatch',
    help: '1 if AVR uses incompatible version of protocol, 0 if OK.'
});

// ==========================================================================================
// Temperature sensor

const temperatureGauge = new Gauge({
    name: 'akua_temperature',
    help: 'Temperature.',
    labelNames: [L_TARGET]
});

const temperatureSamplesGauge = new Gauge({
    name: 'akua_temperature_samples',
    help: 'Temperature samples count.',
    labelNames: [L_TARGET]
});

const temperatureSensorCrcErrorsGauge = new Counter({
    name: 'akua_temperature_sensor_crc_errors',
    help: 'Number of CRC errors during communication with temperature sensor.',
    labelNames: [L_TARGET]
});

const temperatureSensorDisconnectsGauge = new Counter({
    name: 'akua_temperature_sensor_disconnects',
    help: 'Number of time temperature sensor was missing and not replied.',
    labelNames: [L_TARGET]
});

// ==========================================================================================
// Temperature sensor

const co2Gauge = new Gauge({
    name: 'akua_co2',
    help: 'CO2.'
});

const co2SamplesGauge = new Gauge({
    name: 'akua_co2_samples',
    help: 'CO2 samples count.'
});

const co2SensorCrcErrorsGauge = new SettableSimpleCounter({
    name: 'akua_co2_sensor_crc_errors',
    help: 'Number of CRC errors during communication of AVR with CO2 sensor.'
});

const co2SensorRxOverflowsGauge = new SettableSimpleCounter({
    name: 'akua_co2_sensor_rx_overflows',
    help: 'Number of rx overflows during communication of AVR with CO2 sensor.'
});

const co2SensorAbcSetupsGauge = new SettableSimpleCounter({
    name: 'akua_co2_sensor_abc_setups',
    help: 'How many times AVR has turned off/on ABC in CO2 sensor.'
});

const co2SensorTemperatureGauge = new Gauge({
    name: 'akua_co2_sensor_temperature',
    help: 'Temperature of CO2 sensor.'
});

const co2SensorSGauge = new Gauge({
    name: 'akua_co2_sensor_s',
    help: 'Parameter S of MH-Z19 CO2 sensor.'
});

const co2SensorUGauge = new Gauge({
    name: 'akua_co2_sensor_u',
    help: 'Parameter U of MH-Z19 CO2 sensor.'
});

// ==========================================================================================

@injectable()
export default class MetricsServiceImpl extends MetricsService {
    constructor(
        private _avrService: AvrService,
        private _temperatureSensorService: TemperatureSensorService,
        private _co2SensorService: Co2SensorService
    ) {
        super();
    }

    public observeSimpleMeasurement(target: string, delta: [number, number]): void {
        simpleMeasurementSummary.observe({ [L_TARGET]: target }, delta[0] + delta[1] / 1e9);
    }

    public getContentType(): string {
        return register.contentType;
    }

    public getMetrics(): string {
        updateCpuUsageMetrics();
        updateMemUsageMetrics();
        updateLoggingMetrics();

        this._updateServiceMetrics();

        return register.metrics();
    }

    private _updateServiceMetrics() {
        // AVR service
        const avrServiceState = this._avrService.getState();
        avrSerialPortErrorCountGauge.set(avrServiceState.serialPortErrors);
        avrSerialPortOpenAttemptCountGauge.set(avrServiceState.serialPortOpenAttempts);
        avrProtocolCrcErrorCountGauge.set(avrServiceState.protocolCrcErrors);
        avrProtocolDebugMessageCountGauge.set(avrServiceState.protocolDebugMessages);
        avrIncomingMessageCountGauge.set(avrServiceState.incomingMessages);
        avrSerialPortIsOpenGauge.set(avrServiceState.serialPortIsOpen);
        avrProtocolVersionMismatchGauge.set(avrServiceState.protocolVersionMismatch);

        // AVR related stuff
        avrUptimeSecondsGauge.setOrRemove(avrServiceState.lastAvrState?.uptimeSeconds);
        avrUsbRxOverflowsGauge.setOrRemove(avrServiceState.lastAvrState?.usbRxOverflows);
        avrDebugOverflowsGauge.setOrRemove(avrServiceState.lastAvrState?.debugOverflows);

        // TODO: Ignore values in Co2SensorServices if avrTime is too low (< 10 minutes!)
        // TODO: Add last temp/co2 values in addition to average
        // TODO: Only value and valueCount must be part oc Co2 and Temperature. Other stuff must be taken from lastAvrState!
        // TODO: Use setOrRemove below

        // Temperature sensors
        const handleTemperature = (name: string, t: Temperature | null): void => {
            if (t && t.value) {
                temperatureGauge.set({ [L_TARGET]: name }, t.value);
            } else {
                temperatureGauge.remove(name);
            }

            temperatureSensorCrcErrorsGauge.remove(name);
            temperatureSensorDisconnectsGauge.remove(name);

            if (t) {
                temperatureSensorCrcErrorsGauge.inc({ [L_TARGET]: name }, t.crcErrors);
                temperatureSensorDisconnectsGauge.inc({ [L_TARGET]: name }, t.disconnects);
                temperatureSamplesGauge.set({ [L_TARGET]: name }, t.valueSamples);
            }
        };

        handleTemperature("aquarium", this._temperatureSensorService.aquariumTemperature);
        handleTemperature("case", this._temperatureSensorService.caseTemperature);

        // CO2 - - - -
        const co2 = this._co2SensorService.co2;
        if (co2 && co2.value) {
            co2Gauge.set(co2.value);
        } else {
            co2Gauge.remove();
        }

        if (co2) {
            co2SensorCrcErrorsGauge.inc(co2.crcErrors);
            co2SamplesGauge.set(co2.valueSamples);
            co2SensorAbcSetupsGauge.set(co2.abcSetups);
            co2SensorRxOverflowsGauge.set(co2.rxOverflows);
            co2SensorTemperatureGauge.set(co2.temperature);
            co2SensorSGauge.set(co2.s);
            co2SensorUGauge.set(co2.u);
        }
    }
}