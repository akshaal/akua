import { injectable, postConstruct } from "inversify";
import AvrService, { AvrServiceState } from "server/service/AvrService";
import SerialPort from "serialport";
import config from "server/config";
import logger from "server/logger";
import { SerialportReadlineParser } from "./ReadlineParser";
import { avrProtocolVersion } from "server/avr/protocol";

// We do attempt to reopen the port every this number of milliseconds.
const AUTO_REOPEN_MILLIS = 1000;

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

const serialPortOptions: SerialPort.OpenOptions = {
    dataBits: 8,
    parity: 'none',
    stopBits: 1,
    baudRate: 9600,
    autoOpen: false
};

@injectable()
export default class AvrServiceImpl extends AvrService {
    private _serialPort = new SerialPort(config.avr.port, serialPortOptions);
    private _serialPortErrorCount = 0;
    private _serialPortOpenAttemptCount = 0;
    private _protocolCrcErrors = 0;
    private _protocolDebugMessages = 0;
    private _protocolVersionMismatch: 0 | 1 = 0;

    @postConstruct()
    _init(): void {
        this._serialPort.on("error", error => this._onSerialPortError(error));

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
    }

    private _onSerialPortData(data: string): void {
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


    }

    private _onSerialPortError(error: Error): void {
        logger.error("AVR: Serial port error", { error })
        this._serialPortErrorCount += 1;

        // It will be automatically reopened
        if (this._serialPort.isOpen) {
            this._serialPort.close();
        }
    }

    getState(): AvrServiceState {
        return {
            serialPortErrors: this._serialPortErrorCount,
            serialPortOpenAttempts: this._serialPortOpenAttemptCount,
            serialPortIsOpen: this._serialPort.isOpen ? 1 : 0,
            protocolCrcErrors: this._protocolCrcErrors,
            protocolVersionMismatch: this._protocolVersionMismatch,
            protocolDebugMessages: this._protocolDebugMessages,
        };
    }
}