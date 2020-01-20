import "reflect-metadata";
import logger from "./logger";
import config, { asUrl } from "./config";
import createNewServerServicesImpl from "./service_impl/ServerServicesImpl";
import process from "process";
import express, { NextFunction } from "express";
import http from "http";
import bodyParser from 'body-parser';
import { IncomingMessage } from "http";
import { OutgoingMessage } from "http";
import onFinished from "on-finished";

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Starting version ${config.version} in ${config.isProd ? 'production' : 'development'} mode.`);

if (config.isDev) {
    logger.debug("Obtained configuration:", config);
}

logger.debug("next.js app is prepared");

// Create instance of express server
const expressServer = express();
expressServer.set('trust proxy', true);

// Create an instance of server services
const serverServices = createNewServerServicesImpl()

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

expressServer.use("/metrics", measure("/metrics"));

expressServer.get("/metrics", (_, res) => {
    res.set('Content-Type', metricsService.getContentType());
    res.end(metricsService.getMetrics());
});

// -------------------------------------
// Create HTTP server using express framework
http.createServer(expressServer).listen(config.bindOptions, (): void => {
    const bindUrl = asUrl(config.bindOptions);
    logger.info(`Listening on ${bindUrl}.`);
});

