import "reflect-metadata";
import logger from "server/logger";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState } from "server/service/PhPrediction";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import { createCo2ClosingStateFeaturesAndLabels } from "server/service_impl/PhPredictionWorkerThread";
import { writeFileSync } from "fs";
import { realEnv } from "server/env";
import ConfigService from "server/service/ConfigService";

// TODO: Simplify code and make it more convenient... and less boilerplate
// TODO: This code is VERY boilerplate...

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Performing dumping of training data of PH prediction by co2-closing state`);

if (!realEnv.isDev) {
    logger.error("This script must not be started in production mode!");
    exit(-2);
}

const aqua1Container = createNewContainer('cli-utils', { ...realEnv, instanceName: "aqua1" });
const aqua1DatabaseService = aqua1Container.get(DatabaseService);
const aqua1Config = aqua1Container.get(ConfigService).config;

const aqua2Container = createNewContainer('cli-utils', { ...realEnv, instanceName: "aqua2" });
const aqua2DatabaseService = aqua2Container.get(DatabaseService);
const aqua2Config = aqua2Container.get(ConfigService).config;

async function dump() {
    function dumpStates(
        name: string,
        fname: string,
        aqua1States: Readonly<Co2ClosingState[]>,
        aqua2States: Readonly<Co2ClosingState[]>
    ) {
        const statesTotal = aqua1States.length + aqua2States.length;

        console.log(`${name}: aqua1: ${aqua1States.length}, aqua2: ${aqua2States.length}, total: ${statesTotal}\n`);

        const featuresAndLabels = [];

        for (let state of aqua1States) {
            featuresAndLabels.push(createCo2ClosingStateFeaturesAndLabels(state, aqua1Config));
        }

        for (let state of aqua2States) {
            featuresAndLabels.push(createCo2ClosingStateFeaturesAndLabels(state, aqua2Config));
        }

        writeFileSync("../../temp/" + fname, JSON.stringify(featuresAndLabels));
    }

    const aqua1TrainingStates = await aqua1DatabaseService.findCo2ClosingStates(Co2ClosingStateType.TRAINING);
    const aqua1ValidationStates = await aqua1DatabaseService.findCo2ClosingStates(Co2ClosingStateType.VALIDATION);

    const aqua2TrainingStates = await aqua2DatabaseService.findCo2ClosingStates(Co2ClosingStateType.TRAINING);
    const aqua2ValidationStates = await aqua2DatabaseService.findCo2ClosingStates(Co2ClosingStateType.VALIDATION);

    console.log("\n==============================================================");
    dumpStates("Training states", "co2-train-data.json", aqua1TrainingStates, aqua2TrainingStates);
    dumpStates("Validation states", "co2-valid-data.json", aqua1ValidationStates, aqua2ValidationStates);

    exit(0);
}

// ========================================================================================

dump();

