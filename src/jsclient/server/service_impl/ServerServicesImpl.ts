import { Container } from "inversify";
import { InversifyTracer, ReturnInfo, CallInfo } from "inversify-tracer";

import logger from "../logger";
import MetricsServiceImpl from "./MetricsServiceImpl";
import MetricsService from "server/service/MetricsService";
import ServerServices from "server/service/ServerServices";
import DisplayService from "server/service/DisplayService";
import DisplayServiceImpl from "./DisplayServiceImpl";
import AvrService from "server/service/AvrService";
import AvrServiceImpl from "./AvrServiceImpl";
import TemperatureSensorServiceImpl from "./TemperatureSensorServiceImpl";
import TemperatureSensorService from "server/service/TemperatureSensorService";
import Co2SensorServiceImpl from "./Co2SensorServiceImpl";
import Co2SensorService from "server/service/Co2SensorService";

function createNewContainer(): Container {
    const container = new Container();
    container.bind(MetricsService).to(MetricsServiceImpl).inSingletonScope();
    container.bind(DisplayService).to(DisplayServiceImpl).inSingletonScope();
    container.bind(AvrService).to(AvrServiceImpl).inSingletonScope();
    container.bind(TemperatureSensorService).to(TemperatureSensorServiceImpl).inSingletonScope();
    container.bind(Co2SensorService).to(Co2SensorServiceImpl).inSingletonScope();
    container.bind(ServerServices).toSelf().inSingletonScope();

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