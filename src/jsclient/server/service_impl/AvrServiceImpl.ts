import { injectable } from "inversify";
import AvrService, { AvrServiceState } from "server/service/AvrService";
import SerialPort from "serialport";
import config from "server/config";
import logger from "server/logger";
import { SerialportReadlineParser } from "./ReadlineParser";

// We do attempt to reopen the port every this number of milliseconds.
const AUTO_REOPEN_MILLIS = 1000;

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

    constructor() {
        super();

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
            serialPortIsOpen: this._serialPort.isOpen ? 1 : 0
        };
    }
}