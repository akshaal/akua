import { injectable, postConstruct } from "inversify";
import AvrService, { AvrServiceState, AvrState, AvrCo2SensorState, AvrTemperatureSensorState, AvrControlState } from "server/service/AvrService";
import SerialPort from "serialport";
import config from "server/config";
import logger from "server/logger";
import { SerialportReadlineParser } from "./ReadlineParser";
import { avrProtocolVersion, asAvrData, AvrData } from "server/avr/protocol";
import { Subject } from "rxjs";

// We do attempt to reopen the port every this number of milliseconds.
const AUTO_REOPEN_MILLIS = 1000;

// How often we update state to AVR
const AUTO_WRITE_MILLIS = 300;

// ==========================================================================================

function asAvrState(avrData: AvrData): AvrState {
    const co2Sensor: AvrCo2SensorState = {
        updateId: avrData["u8 co2.get_update_id()"],
        crcErrors: avrData["u8 co2.get_crc_errors()"],
        abcSetups: avrData["u16 co2.get_abc_setups()"],
        concentration: avrData["u16 co2.get_concentration()"],
        rawConcentration: avrData["u16 co2.get_raw_concentration()"],
        clampedConcentration: avrData["u16 co2.get_clamped_concentration()"],
        temperature: avrData["u8 co2.get_temperature()"],
        s: avrData["u8 co2.get_s()"],
        u: avrData["u16 co2.get_u()"],
        updatedSecondsAgo: avrData["u8 co2.get_updated_deciseconds_ago()"] / 10.0,
        rxOverflows: avrData["u8 co2.get_rx_overflow_count()"],
    };

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

    return {
        uptimeSeconds: avrData["u32 uptime_deciseconds"] / 10.0,
        debugOverflows: avrData["u8 debug_overflow_count"],
        usbRxOverflows: avrData["u8 usart0_rx_overflow_count"],
        co2Sensor,
        aquariumTemperatureSensor,
        caseTemperatureSensor
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

function serializeControlState(state: AvrControlState): string {
    var result = "";

    function addValue(id: 'D' | 'N', v: number): void {
        const vStr = v ? v + "" : "";

        result += "<";
        result += id;
        result += vStr;
        result += id;
        result += vStr;
        result += ">";
    }

    addValue('D', state.dayLightSwitchOn ? 1 : 0);
    addValue('N', state.nightLightSwitchOn ? 1 : 0);

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

@injectable()
export default class AvrServiceImpl extends AvrService {
    readonly avrState$ = new Subject<AvrState>();
    readonly requestedControlState$ = new Subject<AvrControlState>();

    private _serialPort = new SerialPort(config.avr.port, serialPortOptions);
    private _serialPortErrorCount = 0;
    private _serialPortOpenAttemptCount = 0;
    private _outgoingMessages = 0;
    private _incomingMessages = 0;
    private _protocolCrcErrors = 0;
    private _protocolDebugMessages = 0;
    private _protocolVersionMismatch: 0 | 1 = 0;
    private _lastAvrState?: AvrState;
    private _canWrite = false;
    private _controlStateToWrite?: AvrControlState;

    @postConstruct()
    _init(): void {
        this._serialPort.on("error", error => this._onSerialPortError(error));
        this._serialPort.on("open", () => this._onSerialPortOpen());
        this._serialPort.on("close", () => this._onSerialPortClose());

        // Setup reaction on new data
        const parser = this._serialPort.pipe(new SerialportReadlineParser) as NodeJS.WritableStream;
        parser.on("data", data => this._onSerialPortData(data));

        // Try to open and schedule ourself
        const autoReopen = () => {
            setTimeout(autoReopen, AUTO_REOPEN_MILLIS);
            if (this._serialPort.isOpen) {
                return;
            }
            this._serialPort.open();
            this._serialPortOpenAttemptCount += 1;
        };

        autoReopen();

        // Tries to write new state
        const autoWrite = () => {
            setTimeout(autoWrite, AUTO_WRITE_MILLIS);
            if (this._canWrite) {
                this._write_state();
            }
        };

        autoWrite();
    }

    private _write_state(): void {
        if (!this._canWrite || !this._controlStateToWrite) {
            return;
        }

        this._canWrite = false;

        // Capture state to avoid possible mutation issues
        const controlState = this._controlStateToWrite;
        const text = serializeControlState(controlState);

        logger.debug("Writing");
        this._serialPort.write(text, undefined, () => {
            this._canWrite = true;
            this._outgoingMessages += 1;
            this.requestedControlState$.next(controlState);
            logger.debug("Done writing");
        });
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
            lastAvrState: this._lastAvrState
        };
    }

    public requestControlState(controlState: AvrControlState): void {
        this._controlStateToWrite = controlState;
    }
}