import { injectable, postConstruct } from "inversify";
import { Gauge, register, Summary, Counter } from 'prom-client';
import perfHooks from 'perf_hooks';
import config from "server/config";
import { getInfoCount, getErrorCount, getWarningCount } from "server/logger";
import MetricsService from "server/service/MetricsService";
import AvrService from "server/service/AvrService";
import TemperatureSensorService, { Temperature } from "server/service/TemperatureSensorService";
import Co2SensorService from "server/service/Co2SensorService";
import PhSensorService from "server/service/PhSensorService";

// Use constants for labels to avoid typos and to be consistent about names.
const L_GC_TYPE = 'gc_type';
const L_VERSION = 'version';
const L_TARGET = "target";
const L_LEVEL = "level";

// ==========================================================================================

class SimpleCounter extends Counter {
    constructor(config: { name: string, help: string }) {
        super(config);
    }

    set(value: number) {
        this.remove();
        this.inc(value);
    }

    setOrRemove(value?: number | null) {
        if (typeof value === "number") {
            this.set(value);
        } else {
            this.remove();
        }
    }
}

class SimpleGauge extends Gauge {
    constructor(config: { name: string, help: string }) {
        super(config);
    }

    setOrRemove(value?: number | boolean | null) {
        if (typeof value === "boolean") {
            this.set(value ? 1 : 0);
        } else if (typeof value === "number") {
            this.set(value);
        } else {
            this.remove();
        }
    }
}

class TargetedGauge extends Gauge {
    constructor(config: { name: string, help: string }) {
        super({ ...config, labelNames: [L_TARGET] });
    }

    setOrRemove(target: string, value?: number | null) {
        if (typeof value === "number") {
            this.set({ [L_TARGET]: target }, value);
        } else {
            this.remove(target);
        }
    }
}

class TargetedCounter extends Counter {
    constructor(config: { name: string, help: string }) {
        super({ ...config, labelNames: [L_TARGET] });
    }

    setOrRemove(target: string, value?: number | null) {
        this.remove(target);
        if (typeof value === "number") {
            this.inc({ [L_TARGET]: target }, value);
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

const avrMainLoopDecisecondIterationsSummary = new Summary({
    name: 'akua_avr_main_loop_decisecond_iterations_summary',
    help: "Number of iterations of AVR's main loop within one decisecond.",
    maxAgeSeconds: 60,
    ageBuckets: 5,
    percentiles: [0.1, 0.25, 0.5, 0.75, 0.9, 0.99, 0.999]
});

const avrUptimeSecondsGauge = new SimpleCounter({
    name: 'akua_avr_uptime_seconds',
    help: 'Uptime seconds as returned by AVR (might be inaccurate as there is no RTC there).'
});

const avrClockDriftSecondsGauge = new SimpleGauge({
    name: 'akua_avr_clock_drift_seconds',
    help: 'Drift of AVR clock compared to clock of raspberry pi.'
});

const avrClockCorrectionsSinceProtectionStatResetGauge = new SimpleCounter({
    name: 'akua_avr_clock_corrections_since_protection_stat_reset',
    help: 'Number of times clock has been corrected since statistic was reset (every hour).'
});

const avrClockSecondsSinceMidnightGauge = new SimpleGauge({
    name: 'akua_avr_clock_seconds_since_midnight',
    help: 'Number of seconds since midnight (i.e. clock of AVR).'
});

const avrDebugOverflowsGauge = new SimpleCounter({
    name: 'akua_avr_debug_overflows',
    help: 'Number of times AVR was out of buffer space trying to send debug info to host.'
});

const avrUsbRxOverflowsGauge = new SimpleCounter({
    name: 'akua_avr_usb_rx_overflows',
    help: 'Number of times AVR was out of buffer trying to receive data from USB.'
});

const avrSerialPortErrorCountGauge = new SimpleCounter({
    name: 'akua_avr_serial_port_errors',
    help: 'Number of AVR serial port errors.'
});

const avrSerialPortOpenAttemptCountGauge = new SimpleCounter({
    name: 'akua_avr_serial_port_open_attempts',
    help: 'Number of attempts to open AVR serial port.'
});

const avrProtocolCrcErrorCountGauge = new SimpleCounter({
    name: 'akua_avr_protocol_crc_errors',
    help: 'Number of CRC errors (when decoding incoming message from AVR).'
});

const avrProtocolDebugMessageCountGauge = new SimpleCounter({
    name: 'akua_avr_protocol_debug_messages',
    help: 'Number of messages received from AVR.'
});

const avrIncomingMessageCountGauge = new SimpleCounter({
    name: 'akua_avr_incoming_messages',
    help: 'Total number of messages received from AVR.'
});

const avrOutgoingMessageCountGauge = new SimpleCounter({
    name: 'akua_avr_outgoing_messages',
    help: 'Total number of messages sent to AVR.'
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

const temperatureGauge = new TargetedGauge({
    name: 'akua_temperature',
    help: 'Current average temperature.'
});

const temperatureSamplesGauge = new TargetedGauge({
    name: 'akua_temperature_samples',
    help: 'Current temperature samples count.'
});

const temperatureSensorTemperatureGauge = new TargetedGauge({
    name: 'akua_temperature_sensor_temperature',
    help: 'Last measured temperature as reported by the sensor.'
});

const temperatureSensorUpdatedSecondsAgoGauge = new TargetedGauge({
    name: 'akua_temperature_sensor_updated_seconds_ago',
    help: 'Freshness of last data when it was received from AVR.'
});

const temperatureSensorCrcErrorsGauge = new TargetedCounter({
    name: 'akua_temperature_sensor_crc_errors',
    help: 'Number of CRC errors during communication with temperature sensor.'
});

const temperatureSensorDisconnectsGauge = new TargetedCounter({
    name: 'akua_temperature_sensor_disconnects',
    help: 'Number of time temperature sensor was missing and not replied.'
});

// ==========================================================================================
// CO2 sensor

const co2Gauge = new SimpleGauge({
    name: 'akua_co2',
    help: 'Current average CO2.'
});

const co2SamplesGauge = new SimpleGauge({
    name: 'akua_co2_samples',
    help: 'Current CO2 samples count.'
});

const co2SensorConcentrationGauge = new SimpleGauge({
    name: 'akua_co2_sensor_concentration',
    help: 'Last measured concentration as reported by the sensor.'
});

const co2SensorClampedConcentrationGauge = new SimpleGauge({
    name: 'akua_co2_sensor_clamped_concentration',
    help: 'Last measured clamped concentration as reported by the sensor.'
});

const co2SensorRawConcentrationGauge = new SimpleGauge({
    name: 'akua_co2_sensor_raw_concentration',
    help: 'Last measured raw concentration as reported by the sensor.'
});

const co2SensorUpdatedSecondsAgoGauge = new SimpleGauge({
    name: 'akua_co2_sensor_updated_seconds_ago',
    help: 'Freshness of last data when it was received from AVR.'
});

const co2SensorUptimeSecondsGauge = new SimpleGauge({
    name: 'akua_co2_sensor_uptime_seconds',
    help: 'Number of seconds since the sensor started.'
});

const co2SensorBootGauge = new SimpleGauge({
    name: 'akua_co2_sensor_boot',
    help: '1 means sensor is booting, 0 means it is started.'
});

const co2WarmupGauge = new SimpleGauge({
    name: 'akua_co2_warmup',
    help: '1 means we ignore current co2 values and waiting for sensor or AVR to warm up.'
});

const co2SensorCrcErrorsGauge = new SimpleCounter({
    name: 'akua_co2_sensor_crc_errors',
    help: 'Number of CRC errors during communication of AVR with CO2 sensor.'
});

const co2SensorRxOverflowsGauge = new SimpleCounter({
    name: 'akua_co2_sensor_rx_overflows',
    help: 'Number of rx overflows during communication of AVR with CO2 sensor.'
});

const co2SensorAbcSetupsGauge = new SimpleCounter({
    name: 'akua_co2_sensor_abc_setups',
    help: 'How many times AVR has turned off/on ABC in CO2 sensor.'
});

const co2SensorTemperatureGauge = new SimpleGauge({
    name: 'akua_co2_sensor_temperature',
    help: 'Temperature of CO2 sensor.'
});

const co2SensorSGauge = new SimpleGauge({
    name: 'akua_co2_sensor_s',
    help: 'Parameter S of MH-Z19 CO2 sensor.'
});

const co2SensorUGauge = new SimpleGauge({
    name: 'akua_co2_sensor_u',
    help: 'Parameter U of MH-Z19 CO2 sensor.'
});

// ==========================================================================================
// PH meter

const ph5sVoltageGauge = new SimpleGauge({
    name: 'akua_ph5s_voltage',
    help: 'Current ph averaging 5 seconds of values.'
});

const ph5sVoltageSamplesGauge = new SimpleGauge({
    name: 'akua_ph5s_voltage_samples',
    help: 'Number of samples in ph averaging 5 seconds of values.'
});

const ph60sGauge = new SimpleGauge({
    name: 'akua_ph60s',
    help: 'Current ph averaging 60 seconds of values.'
});

const ph60sSamplesGauge = new SimpleGauge({
    name: 'akua_ph60s_samples',
    help: 'Number of samples in ph averaging 60 seconds of values.'
});

const ph600sGauge = new SimpleGauge({
    name: 'akua_ph600s',
    help: 'Current ph averaging 600 seconds of values.'
});

const phBasedCo2Gauge = new SimpleGauge({
    name: 'akua_ph_based_co2',
    help: 'Current CO2 PPM as calculated from KH and PH values.'
});

const ph600sSamplesGauge = new SimpleGauge({
    name: 'akua_ph600s_samples',
    help: 'Number of samples in ph averaging 600 seconds of values.'
});

const phSensorVoltageGauge = new SimpleGauge({
    name: 'akua_ph_sensor_voltage',
    help: 'Last measured voltage from ph sensor as reported by AVR.'
});

const phSensorVoltageSamplesGauge = new SimpleGauge({
    name: 'akua_ph_sensor_voltage_samples',
    help: 'Number of ADC samples used by AVR to calculate voltage.'
});

// ==========================================================================================

const co2ValveOpenGauge = new SimpleGauge({
    name: 'akua_co2_valve_open',
    help: '1 means open (co2 can flow into tank), 0 means closed (no co2).'
});

const co2ForcedOffGauge = new SimpleGauge({
    name: 'akua_co2_forced_off',
    help: '1 means forced, 0 means not-forced.'
});

const co2RequiredGauge = new SimpleGauge({
    name: 'akua_co2_required',
    help: '1 means that rpi wants CO2 feeded to aquarium, 0 there no need of CO2.'
});

const co2DayGauge = new SimpleGauge({
    name: 'akua_co2_day',
    help: '1 means that it is considered that it is CO2-day now (time when CO2 flow is allowed), 0 means that CO2 can\'t be supplied.'
});

const co2CooldownGauge = new SimpleGauge({
    name: 'akua_co2_cooldown',
    help: 'When CO2 turned off, cooldown timer started. This variable show how many seconds left until cooldown. When it reaches zero, then CO2 can be turned on again if other conditions are met.'
});

const dayLightOnGauge = new SimpleGauge({
    name: 'akua_day_light_on',
    help: '1 means on, 0 means off.'
});

const nightLightOnGauge = new SimpleGauge({
    name: 'akua_night_light_on',
    help: '1 means on, 0 means off.'
});

const dayLightForcedGauge = new SimpleGauge({
    name: 'akua_day_light_forced',
    help: '1 means forced, 0 means not-forced.'
});

const nightLightForcedGauge = new SimpleGauge({
    name: 'akua_night_light_forced',
    help: '1 means forced, 0 means not-forced.'
});

const lightForcesSinceProtectionStatResetGauge = new SimpleGauge({
    name: 'akua_light_forces_since_protection_stat_reset',
    help: 'Number of light forces since last protection stat reset.'
});

// ==========================================================================================

@injectable()
export default class MetricsServiceImpl extends MetricsService {
    constructor(
        private _avrService: AvrService,
        private _temperatureSensorService: TemperatureSensorService,
        private _co2SensorService: Co2SensorService,
        private _phSensorService: PhSensorService
    ) {
        super();
    }

    @postConstruct()
    _init() {
        // Update summaries
        var lastObservedUptime: number | undefined;

        this._avrService.avrState$.subscribe(avrState => {
            // Observe data that's really updated every decisecond
            if (lastObservedUptime != avrState.uptimeSeconds) {
                avrMainLoopDecisecondIterationsSummary.observe(avrState.mainLoopIterationsInLastDecisecond);
                lastObservedUptime = avrState.uptimeSeconds;
            }
        });
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
        const avrServiceState = this._avrService.getServiceState();
        avrSerialPortErrorCountGauge.set(avrServiceState.serialPortErrors);
        avrSerialPortOpenAttemptCountGauge.set(avrServiceState.serialPortOpenAttempts);
        avrProtocolCrcErrorCountGauge.set(avrServiceState.protocolCrcErrors);
        avrProtocolDebugMessageCountGauge.set(avrServiceState.protocolDebugMessages);
        avrIncomingMessageCountGauge.set(avrServiceState.incomingMessages);
        avrOutgoingMessageCountGauge.set(avrServiceState.outgoingMessages);
        avrSerialPortIsOpenGauge.set(avrServiceState.serialPortIsOpen);
        avrProtocolVersionMismatchGauge.set(avrServiceState.protocolVersionMismatch);

        // AVR related stuff
        avrUptimeSecondsGauge.setOrRemove(avrServiceState.lastAvrState?.uptimeSeconds);
        avrUsbRxOverflowsGauge.setOrRemove(avrServiceState.lastAvrState?.usbRxOverflows);
        avrDebugOverflowsGauge.setOrRemove(avrServiceState.lastAvrState?.debugOverflows);
        avrClockCorrectionsSinceProtectionStatResetGauge.setOrRemove(avrServiceState.lastAvrState?.clockCorrectionsSinceProtectionStatReset);
        avrClockDriftSecondsGauge.setOrRemove(avrServiceState.lastAvrState?.clockDriftSeconds);
        avrClockSecondsSinceMidnightGauge.setOrRemove(avrServiceState.lastAvrState?.clockSecondsSinceMidnight);

        // Temperature sensors
        const handleTemperature = (name: string, t: Temperature | null): void => {
            temperatureGauge.setOrRemove(name, t?.value);
            temperatureSamplesGauge.setOrRemove(name, t?.valueSamples);
            temperatureSensorUpdatedSecondsAgoGauge.setOrRemove(name, t?.lastSensorState?.updatedSecondsAgo);
            temperatureSensorTemperatureGauge.setOrRemove(name, t?.lastSensorState?.temperature);
            temperatureSensorCrcErrorsGauge.setOrRemove(name, t?.lastSensorState?.crcErrors);
            temperatureSensorDisconnectsGauge.setOrRemove(name, t?.lastSensorState?.disconnects);
        };

        handleTemperature("aquarium", this._temperatureSensorService.aquariumTemperature);
        handleTemperature("case", this._temperatureSensorService.caseTemperature);

        // CO2 - - - -
        const co2 = this._co2SensorService.co2;
        co2Gauge.setOrRemove(co2?.value);
        co2SamplesGauge.setOrRemove(co2?.valueSamples);
        co2WarmupGauge.setOrRemove(co2?.warmup);
        co2SensorBootGauge.setOrRemove(co2?.sensorBoot);
        co2SensorUptimeSecondsGauge.setOrRemove(co2?.sensorUptimeSeconds);
        co2SensorConcentrationGauge.setOrRemove(co2?.lastSensorState?.concentration);
        co2SensorRawConcentrationGauge.setOrRemove(co2?.lastSensorState?.rawConcentration);
        co2SensorClampedConcentrationGauge.setOrRemove(co2?.lastSensorState?.clampedConcentration);
        co2SensorUpdatedSecondsAgoGauge.setOrRemove(co2?.lastSensorState?.updatedSecondsAgo);
        co2SensorCrcErrorsGauge.setOrRemove(co2?.lastSensorState?.crcErrors);
        co2SensorAbcSetupsGauge.setOrRemove(co2?.lastSensorState?.abcSetups);
        co2SensorRxOverflowsGauge.setOrRemove(co2?.lastSensorState?.rxOverflows);
        co2SensorTemperatureGauge.setOrRemove(co2?.lastSensorState?.temperature);
        co2SensorSGauge.setOrRemove(co2?.lastSensorState?.s);
        co2SensorUGauge.setOrRemove(co2?.lastSensorState?.u);
        co2ValveOpenGauge.setOrRemove(avrServiceState.lastAvrState?.co2ValveOpen);
        co2CooldownGauge.setOrRemove(avrServiceState.lastAvrState?.co2CooldownSeconds);
        co2DayGauge.setOrRemove(avrServiceState.lastAvrState?.co2day);
        co2ForcedOffGauge.setOrRemove(avrServiceState.lastAvrState?.co2forcedOff);
        co2RequiredGauge.setOrRemove(avrServiceState.lastAvrState?.co2IsRequired);

        // Light - - - - 
        const light = avrServiceState.lastAvrState?.light;
        dayLightOnGauge.setOrRemove(light?.dayLightOn);
        nightLightOnGauge.setOrRemove(light?.nightLightOn);
        dayLightForcedGauge.setOrRemove(light?.dayLightForced);
        nightLightForcedGauge.setOrRemove(light?.nightLightForced);
        lightForcesSinceProtectionStatResetGauge.setOrRemove(light?.lightForcesSinceProtectionStatReset);

        // Ph - - - - - - - - - -
        const ph = this._phSensorService.ph;
        ph60sGauge.setOrRemove(ph?.value60s);
        ph60sSamplesGauge.setOrRemove(ph?.value60sSamples);
        ph600sGauge.setOrRemove(ph?.value600s);
        ph600sSamplesGauge.setOrRemove(ph?.value600sSamples);
        ph5sVoltageGauge.setOrRemove(ph?.voltage5s);
        ph5sVoltageSamplesGauge.setOrRemove(ph?.voltage5sSamples);
        phSensorVoltageGauge.setOrRemove(ph?.lastSensorState?.voltage);
        phSensorVoltageSamplesGauge.setOrRemove(ph?.lastSensorState?.voltageSamples);
        phBasedCo2Gauge.setOrRemove(ph?.phBasedCo2);
    }
}