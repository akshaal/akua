import "reflect-metadata";
import logger from "./logger";
import process from "process";
import express, { NextFunction } from "express";
import http from "http";
import bodyParser from 'body-parser';
import { IncomingMessage } from "http";
import { OutgoingMessage } from "http";
import onFinished from "on-finished";
import { LightForceMode } from "./service/AvrService";
import { createNewContainer } from "./service_impl/ServerServicesImpl";
import ServerServices from "./service/ServerServices";
import ConfigService from "./service/ConfigService";
import { realEnv } from "./env";
import { asUrl } from "./misc/asUrl";
import { timer } from "rxjs";
import { getElapsedSecondsSince } from "./misc/get-elapsed-seconds-since";

const DIE_IF_NO_AVR_LIVE_LAST_THIS_MINUTES = 30;

// Create an instance of server services
const container = createNewContainer('express-server', realEnv);

// TODO: Convert everything below this comment into a service and test it!
const serverServices = container.get(ServerServices);
const config = container.get(ConfigService).config;

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Starting version ${config.version} in ${config.isDev ? 'development' : 'production'} mode on instance ${config.instanceName} with instance-id ${config.instanceId}.`);

if (config.isDev) {
    logger.debug("Obtained configuration:", config);
}

logger.debug("next.js app is prepared");

// Create instance of express server
const expressServer = express();
expressServer.set('trust proxy', true);

// Our custom middleware to measure requests in express.
function measure(target: string) {
    return function (_: IncomingMessage, res: OutgoingMessage, next: NextFunction) {
        const started = process.hrtime();

        onFinished(res, () => {
            const delta = process.hrtime(started);
            serverServices.metricsService.observeSimpleMeasurement(target, delta);
        });

        next();
    };
}

// -----------------------------------
// Parse JSON-body

expressServer.use(bodyParser.urlencoded({ extended: true }));

// -------------------------------------
// Metrics

const metricsService = serverServices.metricsService;
const avrService = serverServices.avrService;

expressServer.use("/metrics", measure("/metrics"));

expressServer.get("/metrics", (_, res) => {
    res.set('Content-Type', metricsService.getContentType());
    res.end(metricsService.getMetrics());
});

// -------------------------------------
// Shitty watchdog, pi has problems.. sometimes it stops reading from ports until you reopen the port...

let lastAvrReadHrTime = process.hrtime();
serverServices.avrService.avrState$.subscribe(() => {
    lastAvrReadHrTime = process.hrtime();
});

timer(1000, 1000).subscribe(() => {
    const elapsedSecs = getElapsedSecondsSince({
        now: process.hrtime(),
        since: lastAvrReadHrTime,
    });
    const elapsedMinutes = elapsedSecs / 60.0;

    if (elapsedMinutes >= DIE_IF_NO_AVR_LIVE_LAST_THIS_MINUTES) {
        logger.error(`No data from AVR in ${DIE_IF_NO_AVR_LIVE_LAST_THIS_MINUTES} minutes! Dying..`);
        process.exit(200);
    }
});

// -------------------------------------
// Light commands

expressServer.get("/force-day-light", (_, res) => {
    avrService.forceLight(LightForceMode.Day);
    res.end("OK");
});

expressServer.get("/force-night-light", (_, res) => {
    avrService.forceLight(LightForceMode.Night);
    res.end("OK");
});

expressServer.get("/do-not-force-light", (_, res) => {
    avrService.forceLight(LightForceMode.NotForced);
    res.end("OK");
});

// -------------------------------------
// CO2 COMMANDS

expressServer.get("/force-co2-off", (_, res) => {
    avrService.forceCo2Off();
    res.end("OK");
});

// -------------------------------------
// SIMPLE UI

expressServer.use('/ui', express.static('server/static-ui'));
expressServer.use('/db', express.static(config.database.baseDirectory));

// -------------------------------------
// Create HTTP server using express framework
http.createServer(expressServer).listen(config.bindOptions, (): void => {
    const bindUrl = asUrl(config.bindOptions);
    logger.info(`Listening on ${bindUrl}.`);
});

