import { Container } from "inversify";

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
import PhPredictionServiceImpl from "./PhPredictionServiceImpl";
import PhPredictionService from "server/service/PhPredictionService";
import TimeServiceImpl from "./TimeServiceImpl";
import TimeService from "server/service/TimeService";
import DatabaseService from "server/service/DatabaseService";
import DatabaseServiceImpl from "./DatabaseServiceImpl";
import logger from "server/logger";
import RandomNumberService from "server/service/RandomNumberService";
import RandomNumberServiceImpl from "./RandomNumberServiceImpl";
import { Env, ENV_IOC_TOKEN } from "server/env";
import ConfigServiceImpl from "./ConfigServiceImpl";
import ConfigService from "server/service/ConfigService";

type ContainerMode = 'cli-utils' | 'express-server';

export function createNewContainer(mode: ContainerMode, env: Env): Container {
    const container = new Container();

    container.bind(ENV_IOC_TOKEN).toConstantValue(env);
    container.bind(TimeService).to(TimeServiceImpl).inSingletonScope();
    container.bind(RandomNumberService).to(RandomNumberServiceImpl).inSingletonScope();
    container.bind(DatabaseService).to(DatabaseServiceImpl).inSingletonScope();
    container.bind(ConfigService).to(ConfigServiceImpl).inSingletonScope();

    logger.info("Injection container created in '" + mode + "' mode.");

    if (mode == 'express-server') {
        container.bind(MetricsService).to(MetricsServiceImpl).inSingletonScope();
        container.bind(DisplayService).to(DisplayServiceImpl).inSingletonScope();
        container.bind(DisplayManagerService).to(DisplayManagerServiceImpl).inSingletonScope();
        container.bind(AvrService).to(AvrServiceImpl).inSingletonScope();
        container.bind(TemperatureSensorService).to(TemperatureSensorServiceImpl).inSingletonScope();
        container.bind(PhSensorService).to(PhSensorServiceImpl).inSingletonScope();
        container.bind(PhPredictionService).to(PhPredictionServiceImpl).inSingletonScope();
        container.bind(Co2ControllerService).to(Co2ControllerServiceImpl).inSingletonScope();
        container.bind(ServerServices).toSelf().inSingletonScope();
    }

    return container;
}

