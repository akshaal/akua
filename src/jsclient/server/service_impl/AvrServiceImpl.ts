import { injectable, postConstruct } from "inversify";
import AvrService, { AvrServiceState, AvrState, AvrTemperatureSensorState, LightForceMode, AvrLightState, Co2ValveOpenState, AvrPhState } from "server/service/AvrService";
import SerialPort from "serialport";
import logger from "server/logger";
import { SerialportReadlineParser } from "./ReadlineParser";
import { avrProtocolVersion, asAvrData, AvrData } from "server/avr/protocol";
import { Subject } from "rxjs";
import { recurrent } from "../misc/recurrent";
import ConfigService from "server/service/ConfigService";

// We do attempt to reopen the port every this number of milliseconds.
const AUTO_REOPEN_MILLIS = 1000;

// How often we update state to AVR
const AUTO_WRITE_MILLIS = 100;

// How often we send our clock time to AVR
const CLOCK_UPDATE_MILLIS = 3000;

// ==========================================================================================

function asAvrState(avrData: AvrData): AvrState {
    const aquariumTemperatureSensor: AvrTemperatureSensorState = {
        updateId: avrData["u8 ds18b20_aqua.get_update_id()"],
        crcErrors: avrData["u8 ds18b20_aqua.get_crc_errors()"],
        disconnects: avrData["u8 ds18b20_aqua.get_disconnects()"],
        temperature: avrData["u16 ds18b20_aqua.get_temperatureX16()"] / 16.0,
        updatedSecondsAgo: avrData["u8 ds18b20_aqua.get_updated_deciseconds_ago()"] / 10.0,
    };

    const caseTemperatureSensor: AvrTemperatureSensorState = {
        updateId: avrData["u8 ds18b20_case.get_update_id()"],
        crcErrors: avrData["u8 ds18b20_case.get_crc_errors()"],
        disconnects: avrData["u8 ds18b20_case.get_disconnects()"],
        temperature: avrData["u16 ds18b20_case.get_temperatureX16()"] / 16.0,
        updatedSecondsAgo: avrData["u8 ds18b20_case.get_updated_deciseconds_ago()"] / 10.0,
    };

    const ph: AvrPhState = {
        voltage: avrData["u32 __ph_adc_accum"] / (avrData["u16 __ph_adc_accum_samples"] || 1) * 5.0 / 1024.0,
        voltageSamples: avrData["u16 __ph_adc_accum_samples"],
        badSamples: avrData["u16 __ph_adc_bad_samples"]
    };

    const light: AvrLightState = {
        dayLightOn: !!avrData["u8 day_light_switch.is_set() ? 1 : 0"],
        nightLightOn: !!avrData["u8 night_light_switch.is_set() ? 1 : 0"],
        dayLightForced: !!avrData["u8 day_light_forced.is_set() ? 1 : 0"],
        nightLightForced: !!avrData["u8 night_light_forced.is_set() ? 1 : 0"],
        lightForcesSinceProtectionStatReset: avrData["u8 light_forces_since_protection_stat_reset"],
        alternativeDayEnabled: !!avrData["u8 alternative_day_enabled"],
    };

    let clockDriftSeconds = avrData["u32 ((u32)last_drift_of_clock_deciseconds_since_midnight)"];
    if (clockDriftSeconds > 0x7FFFFFFF) {
        clockDriftSeconds = clockDriftSeconds - 0x100000000;
    }
    clockDriftSeconds = clockDriftSeconds / 10.0;

    return {
        uptimeSeconds: avrData["u32 uptime_deciseconds"] / 10.0,
        clockDriftSeconds,
        clockCorrectionsSinceProtectionStatReset: avrData["u32 clock_corrections_since_protection_stat_reset"],
        clockSecondsSinceMidnight: avrData["u32 clock_deciseconds_since_midnight"] / 10.0,
        mainLoopIterationsInLastDecisecond: avrData["u32 main_loop_iterations_in_last_decisecond"],
        debugOverflows: avrData["u8 debug_overflow_count"],
        usbRxOverflows: avrData["u8 usart0_rx_overflow_count"],
        co2ValveOpen: !!avrData["u8 co2_switch.is_set() ? 1 : 0"],
        co2CooldownSeconds: avrData["u32 co2_deciseconds_until_can_turn_on"] / 10,
        co2IsRequired: !!avrData["u8 required_co2_switch_state.is_set() ? 1 : 0"],
        co2day: !!avrData["u8 co2_calculated_day ? 1 : 0"],
        co2forcedOff: !!avrData["u8 co2_force_off.is_set() ? 1 : 0"],
        aquariumTemperatureSensor,
        caseTemperatureSensor,
        light,
        ph
    };
}

// ==========================================================================================

function addCrc(crc: number, byte: number): number {
    for (let j = 0; j < 8; j++) {
        let m = (crc ^ byte) & 1;
        crc >>= 1;
        if (m) {
            crc ^= 0x8C;
        }
        byte >>= 1;
    }

    return crc;
}

function calcCrc(str: string): number {
    let crc = 0;
    for (let i = 0; i < str.length; i++) {
        crc = addCrc(crc, str.charCodeAt(i));
    }
    return crc;
}

// ==========================================================================================

function serializeCommands(commands: {
    lightForceMode?: LightForceMode,
    newCo2ValveOpenState?: Co2ValveOpenState,
    co2ForceOff?: boolean,
    sendClock: boolean,
    altDayEnabled: boolean
}): string {
    var result = "";

    function addValue(id: 'L' | 'A' | 'B' | 'C' | 'D' | 'G' | 'F', v?: number): void {
        if (typeof v === "undefined") {
            return;
        }

        const vStr = v ? v + "" : "";

        result += "<";
        result += id;
        result += vStr;
        result += id;
        result += vStr;
        result += ">";
    }

    addValue('L', commands.lightForceMode);
    addValue('G', commands.newCo2ValveOpenState);
    addValue('F', commands.co2ForceOff === true ? 1 : undefined);
    addValue('D', commands.altDayEnabled ? 1 : undefined);

    if (commands.sendClock) {
        // Order of commands is important!
        const d = new Date();
        const c = ((d.getHours() * 60 + d.getMinutes()) * 60 + d.getSeconds()) * 10 + Math.floor(d.getMilliseconds() / 100.0);
        addValue('A', c % 256);
        addValue('B', Math.floor(c / 256) % 256);
        addValue('C', Math.floor(c / 65536));
    }

    return result;
}

// ==========================================================================================

const serialPortOptions: SerialPort.OpenOptions = {
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    baudRate: 9600,
    autoOpen: false
};

// ==========================================================================================

@injectable()
export default class AvrServiceImpl extends AvrService {
    readonly avrState$ = new Subject<AvrState>();

    private _serialPort = new SerialPort(this._configService.config.avr.port, serialPortOptions);
    private _serialPortErrorCount = 0;
    private _serialPortOpenAttemptCount = 0;
    private _outgoingMessages = 0;
    private _incomingMessages = 0;
    private _protocolCrcErrors = 0;
    private _protocolDebugMessages = 0;
    private _protocolVersionMismatch: 0 | 1 = 0;
    private _lastAvrState?: AvrState;
    private _canWrite = false;
    private _lightForceMode?: LightForceMode;
    private _sendClockReq: boolean = false;
    private _newCo2RequiredValveOpenState?: Co2ValveOpenState;
    private _forceCo2Off?: boolean;

    constructor(private readonly _configService: ConfigService) {
        super();
    }

    @postConstruct()
    _init(): void {
        this._serialPort.on("error", error => this._onSerialPortError(error));
        this._serialPort.on("open", () => this._onSerialPortOpen());
        this._serialPort.on("close", () => this._onSerialPortClose());

        // Setup reaction on new data
        const parser = this._serialPort.pipe(new SerialportReadlineParser) as NodeJS.WritableStream;
        parser.on("data", data => this._onSerialPortData(data));

        // Tries to open port if closed
        recurrent(AUTO_REOPEN_MILLIS, () => {
            if (this._serialPort.isOpen) {
                return;
            }
            this._serialPort.open();
            this._serialPortOpenAttemptCount += 1;
        });

        // Send commands to AVR
        recurrent(AUTO_WRITE_MILLIS, () => this._write_commands());

        // Send clock
        recurrent(CLOCK_UPDATE_MILLIS, () => this._sendClockReq = true);
    }

    // Write commands if needed, this is called recurrently
    private _write_commands(): void {
        if (!this._canWrite) {
            return;
        }

        // Create commands, this will return empty string if no commands needed
        const text = serializeCommands({
            lightForceMode: this._lightForceMode,
            sendClock: this._sendClockReq,
            newCo2ValveOpenState: this._newCo2RequiredValveOpenState,
            co2ForceOff: this._forceCo2Off,
            altDayEnabled: this._configService.config.aquaEnv.alternativeDay
        });

        // Don't try to write if there is nothing to write
        if (text == "") {
            return;
        }

        // Assume that we have sent it...
        this._forceCo2Off = undefined;
        this._lightForceMode = undefined;
        this._newCo2RequiredValveOpenState = undefined;
        this._sendClockReq = false;

        // Set us into busy mode
        this._canWrite = false;

        // Actually write
        logger.debug("Writing");
        this._serialPort.write(text, undefined, () => {
            this._canWrite = true;
            this._outgoingMessages += 1;
            logger.debug("Done writing");
        });
        this._serialPort.drain();
    }

    private _onSerialPortOpen(): void {
        logger.debug("Open");
        this._canWrite = true;
    }

    private _onSerialPortClose(): void {
        logger.debug("Close");
        this._canWrite = false;
    }

    private _onSerialPortData(data: string): void {
        this._incomingMessages += 1;

        data = (data || "").replace("\r", "");

        if (data.indexOf('>') >= 0) {
            logger.warn("AVR: Protocol debug: " + data);
            this._protocolDebugMessages += 1;
            return;
        }

        const fields = data.split(" ");

        if (fields.length < 4 || fields[0] != '') {
            // Doesn't look sane
            this._protocolCrcErrors += 1;
            return;
        }

        function parseHex(str: string): number {
            return parseInt("0x" + (str || "0"));
        }

        // Check CRC
        const crcField = fields[fields.length - 1];
        const crc = parseHex(crcField);
        const crcSubject = data.substr(0, data.length - crcField.length)
        const calculatedCrc = calcCrc(crcSubject);

        if (calculatedCrc != crc) {
            logger.debug("AVR: Wrong CRC", { crc, calculatedCrc, crcSubject })
            this._protocolCrcErrors += 1;
            return;
        }

        // Check version
        const version = parseHex(fields[fields.length - 2]);
        this._protocolVersionMismatch = version != avrProtocolVersion ? 1 : 0;
        if (this._protocolVersionMismatch) {
            logger.debug("AVR: Protocol version mismatch", { version, avrProtocolVersion })
            return;
        }

        // Parse fields
        const vals: { [id: string]: number } = {};
        for (let fieldIdx = 1; fieldIdx < fields.length - 2; fieldIdx++) {
            const field = fields[fieldIdx];
            const prefix = field[0];
            const valStrings = field.substr(1).split(",");
            for (let valIdx = 0; valIdx < valStrings.length; valIdx++) {
                vals[prefix + (valIdx + 1)] = parseHex(valStrings[valIdx]);
            }
        }

        logger.debug("AVR: parsed values", { vals });

        // Convert into more meaningful and stable AvrData structure
        const avrData = asAvrData(vals);
        logger.debug("AVR: parsed data", { avrData });

        // Convert into AvrState and publish
        const avrState = asAvrState(avrData);
        logger.debug("AVR: next AvrSate", { avrState });

        this._lastAvrState = avrState;
        this.avrState$.next(avrState);
    }

    private _onSerialPortError(error: Error): void {
        logger.error("AVR: Serial port error", { error })
        this._serialPortErrorCount += 1;
        this._canWrite = false;

        // It will be automatically reopened
        if (this._serialPort.isOpen) {
            this._serialPort.close();
        }
    }

    public getServiceState(): AvrServiceState {
        return {
            serialPortErrors: this._serialPortErrorCount,
            serialPortOpenAttempts: this._serialPortOpenAttemptCount,
            serialPortIsOpen: this._serialPort.isOpen ? 1 : 0,
            protocolCrcErrors: this._protocolCrcErrors,
            protocolVersionMismatch: this._protocolVersionMismatch,
            protocolDebugMessages: this._protocolDebugMessages,
            incomingMessages: this._incomingMessages,
            outgoingMessages: this._outgoingMessages,
            lastAvrState: this._lastAvrState,
        };
    }

    public forceLight(mode: LightForceMode): void {
        logger.info("Forcing light mode: " + mode);
        this._lightForceMode = mode;
    }

    public setCo2RequiredValveOpenState(newCo2RequiredValveOpenState: Co2ValveOpenState): void {
        this._newCo2RequiredValveOpenState = newCo2RequiredValveOpenState;
    }

    public forceCo2Off(): void {
        this._forceCo2Off = true;
    }
}