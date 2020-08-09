import util from "util";
import winston from "winston";
import process from "process";
import Transport from 'winston-transport';
import { LEVEL } from "triple-beam";
import { realEnv } from "./env";

var infos = 0;
var warnings = 0;
var errors = 0;

class CountingTransport extends Transport {
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    public log?(info: { [key: string]: any }, next: () => void): void {
        switch (info[LEVEL]) {
            case "info":
                infos += 1;
                break;

            case "warn":
                warnings += 1;
                break;

            case "error":
                errors += 1;
                break;

            default:
        }

        if (next) {
            next();
        }
    }
}

new CountingTransport().on

function formatArgs(args: { [key: string]: unknown }): string {
    const gargs: { [key: string]: unknown } = {};
    for (const k in args) {
        gargs[k] = args[k];
    }

    const s = util.inspect(gargs);
    return s === "{}" ? "" : s;
}

const logger: winston.Logger = winston.createLogger({
    transports: [
        new CountingTransport(),

        new winston.transports.Console({
            level: 'info',
            handleExceptions: false, // We use our own handler
            format: winston.format.combine(
                winston.format.colorize(),
                winston.format.timestamp(),
                winston.format.align(),
                winston.format.printf((info): string => {
                    const { timestamp, level, message, ...args } = info;
                    const ts = timestamp.replace('T', ' ');
                    return `${ts} [${level}]: ${message} ${formatArgs(args)}`;
                }),
            )
        })
    ],

    // do not exit on handled exceptions
    exitOnError: false,
});

if (realEnv.isDev) {
    logger.add(
        new winston.transports.File({
            level: 'debug',
            filename: `./logs/app-debug.log`,
            handleExceptions: false, // We use our own handler
            maxsize: 10 * 1024 * 1024,
            maxFiles: 10,
            format: winston.format.combine(
                winston.format.timestamp(),
                winston.format.align(),
                winston.format.printf((info): string => {
                    const { timestamp, level, message, ...args } = info;
                    const ts = timestamp.replace('T', ' ');
                    return `${ts} [${level}]: ${message} ${formatArgs(args)}`;
                }),
            )
        })
    );
}

process.on('uncaughtException', (err) => {
    logger.error(`Uncaught: ${err.stack}`);
    process.exit(1); //mandatory (as per the Node docs)
});

export function getErrorCount() {
    return errors;
}

export function getWarningCount() {
    return warnings;
}

export function getInfoCount() {
    return infos;
}

export default logger;
