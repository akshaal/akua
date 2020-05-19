import "reflect-metadata";
import logger from "server/logger";
import config from "server/config";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState } from "server/service/PhPrediction";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import * as tf from 'server/service_impl/tf';
import { createCo2ClosingStateFeaturesAndLabels } from "server/service_impl/PhPredictionWorkerThread";
import { writeFileSync } from "fs";

// TODO: Need comments and cleanup!

// TODO: Use config!
const minPhPredictionModelLocationForCli = 'file://server/static-ui/model.dump';

type Co2ClosingStateTfData = { xs: tf.Tensor1D, ys: tf.Tensor1D };
type Co2ClosingStateTfDataset = tf.data.Dataset<Co2ClosingStateTfData>;

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Performing training of ph2 prediction by co2-closing state`);

if (!config.isDev) {
    logger.error("This script must not be started in production mode!");
    exit(-2);
}

const container = createNewContainer('cli-utils');
const databaseService = container.get(DatabaseService);

function sleep(ms: number) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}

// ---------------------------------------------------------------

function prepareCo2ClosingStateTfDataset(states: Readonly<Co2ClosingState[]>): Co2ClosingStateTfDataset {
    const dataArray: Co2ClosingStateTfData[] = [];

    for (const state of states) {
        const featuresAndLabels = createCo2ClosingStateFeaturesAndLabels(state);
        if (!featuresAndLabels) {
            continue;
        }

        dataArray.push({
            xs: tf.tensor1d(featuresAndLabels.xs),
            ys: tf.tensor1d(featuresAndLabels.ys),
        });
    }

    return tf.data.array(dataArray).shuffle(1000);
}

// ---------------------------------------------------------------

export async function trainModelFromDataset(
    params: {
        retrain: boolean,
        trainingStates: Readonly<Co2ClosingState[]>,
        validationStates: Readonly<Co2ClosingState[]>
    }
) {
    // Train

    // TODO: Cleanup.... and describe and so on

    // TODO: Crappy code... tune batch sizes and stuff... move it to prepare code... blah blah blah
    const trainDataset = prepareCo2ClosingStateTfDataset(params.trainingStates).batch(params.trainingStates.length).prefetch(1);
    const validDataset = prepareCo2ClosingStateTfDataset(params.validationStates).batch(params.validationStates.length).prefetch(1);

    const learningRate = 5e-4;
    //const learningRate = undefined;

    const optimizer = tf.train.adam(learningRate);

    console.log("Optimizer config: ", optimizer.getConfig());

    var model;

    if (params.retrain) {
        logger.info("Doing retrain");

        model = tf.sequential({
            layers: [
                tf.layers.dense({ units: 2, inputDim: 45, activation: "selu", kernelInitializer: 'leCunNormal' }),
                tf.layers.dense({ units: 1, activation: "selu", kernelInitializer: 'leCunNormal' })
            ]
        });

    } else {
        logger.info("Training existing model");
        model = await tf.loadLayersModel(minPhPredictionModelLocationForCli + "/model.json");
    }

    model.compile({ loss: "meanSquaredError", optimizer });

    const result = await model.fitDataset(trainDataset, { epochs: 400_000, verbose: 1, validationData: validDataset });

    await model.save(minPhPredictionModelLocationForCli);

    // Save learning history to plot it later
    var historyFileContent = "";

    for (var epoch of result.epoch) {
        const loss = result.history.loss[epoch];
        const val_loss = result.history.val_loss[epoch];
        historyFileContent += `${epoch} ${loss} ${val_loss}\n`;
    }

    writeFileSync("../../temp/train-co2-log.dat", historyFileContent);
}

// ---------------------------------------------------------------

async function train() {
    const trainingStates = await databaseService.findCo2ClosingStates(Co2ClosingStateType.TRAINING);
    const validationStates = await databaseService.findCo2ClosingStates(Co2ClosingStateType.VALIDATION);

    function dumpStates(name: string, states: Readonly<Co2ClosingState[]>) {
        console.log("\n==============================================================");
        console.log(name + "\n");

        for (var state of states) {
            const date = new Date(state.closeTime * 1000).toLocaleString("nb");
            console.log(date + ": Offset after close: " + state.minPh600OffsetAfterClose + "  (i.e. " + (state.minPh600OffsetAfterClose + state.ph600AtClose) + ")");
        }
    }

    dumpStates("Training states", trainingStates);
    dumpStates("Validation states", validationStates);

    console.log("Training states: " + trainingStates.length + ".  Validation states: " + validationStates.length + ".");

    await sleep(3000);

    await trainModelFromDataset({
        trainingStates,
        validationStates,
        retrain: false
    });

    exit(0);
}

// ========================================================================================

train();
