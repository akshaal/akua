import "reflect-metadata";
import logger from "server/logger";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState } from "server/service/PhPrediction";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import { createCo2ClosingStateFeaturesAndLabels } from "server/service_impl/PhPredictionWorkerThread";
import { writeFileSync } from "fs";
import { realEnv } from "server/env";

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Performing dumping of training data of PH prediction by co2-closing state`);

if (!realEnv.isDev) {
    logger.error("This script must not be started in production mode!");
    exit(-2);
}

const container = createNewContainer('cli-utils', realEnv);
const databaseService = container.get(DatabaseService);

async function dump() {
    const trainingStates = await databaseService.findCo2ClosingStates(Co2ClosingStateType.TRAINING);
    const validationStates = await databaseService.findCo2ClosingStates(Co2ClosingStateType.VALIDATION);

    function dumpStates(name: string, fname: string, states: Readonly<Co2ClosingState[]>) {
        console.log("\n==============================================================");
        console.log(name + "\n");

        const featuresAndLabels = [];

        for (var state of states) {
            const date = new Date(state.closeTime * 1000).toLocaleString("nb");
            console.log(date + ": Offset after close: " + state.minPh600OffsetAfterClose + "  (i.e. " + (state.minPh600OffsetAfterClose + state.ph600AtClose) + ")");

            featuresAndLabels.push(createCo2ClosingStateFeaturesAndLabels(state));
        }

        writeFileSync("../../temp/" + fname, JSON.stringify(featuresAndLabels));
    }

    dumpStates("Training states", "co2-train-data.json", trainingStates);
    dumpStates("Validation states", "co2-valid-data.json", validationStates);

    exit(0);
}

// ========================================================================================

dump();

