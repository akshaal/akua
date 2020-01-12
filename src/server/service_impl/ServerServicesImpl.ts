import { Container } from "inversify";
import { InversifyTracer, ReturnInfo, CallInfo } from "inversify-tracer";

import logger from "../logger";
import MetricsServiceImpl from "./MetricsServiceImpl";
import MetricsService from "server/service/MetricsService";
import ServerServices from "server/service/ServerServices";

function createNewContainer(): Container {
    const container = new Container();
    container.bind(MetricsService).to(MetricsServiceImpl);

    if (process.env.NODE_ENV === "development") {
        const tracer = new InversifyTracer();

        tracer.on('call', (callInfo: CallInfo) => {
            logger.debug(`${callInfo.className} ${callInfo.methodName} called with: `, callInfo.parameters);
        });

        tracer.on('return', (returnInfo: ReturnInfo) => {
            logger.debug(`${returnInfo.className} ${returnInfo.methodName} returned in ${returnInfo.executionTime}ms:`, returnInfo.result);
        });

        tracer.apply(container);
    }

    return container;
}

export default function createNewInstance(): ServerServices {
    return createNewContainer().get(ServerServices);
}