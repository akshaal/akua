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
import PhSensorService from "server/service/PhSensorService";
import PhSensorServiceImpl from "./PhSensorServiceImpl";
import DisplayManagerServiceImpl from "./DisplayManagerServiceImpl";
import DisplayManagerService from "server/service/DisplayManagerService";
import Co2ControllerServiceImpl from "./Co2ControllerServiceImpl";
import Co2ControllerService from "server/service/Co2ControllerService";

function createNewContainer(): Container {
    const container = new Container();
    container.bind(MetricsService).to(MetricsServiceImpl).inSingletonScope();
    container.bind(DisplayService).to(DisplayServiceImpl).inSingletonScope();
    container.bind(DisplayManagerService).to(DisplayManagerServiceImpl).inSingletonScope();
    container.bind(AvrService).to(AvrServiceImpl).inSingletonScope();
    container.bind(TemperatureSensorService).to(TemperatureSensorServiceImpl).inSingletonScope();
    container.bind(PhSensorService).to(PhSensorServiceImpl).inSingletonScope();
    container.bind(Co2ControllerService).to(Co2ControllerServiceImpl).inSingletonScope();
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