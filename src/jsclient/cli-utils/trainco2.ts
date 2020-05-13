import "reflect-metadata";
import logger from "server/logger";
import config from "server/config";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState } from "server/service/PhPrediction";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import * as tf from 'server/service_impl/tf';
import { createCo2ClosingStateFeaturesAndLabels, loadModelFromFile } from "server/service_impl/PhPredictionWorkerThread";

// TODO: Need comments and cleanup!

// TODO: Use config!
const minPhPredictionModelSaveLocation = 'file://server/static-ui/model.dump';

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

    console.log("Dataset size: ", dataArray.length);

    return tf.data.array(dataArray).shuffle(1000);
}

// ---------------------------------------------------------------

export async function trainModelFromDataset(states: Readonly<Co2ClosingState[]>, params: { retrain: boolean }) {
    // Train

    // TODO: Cleanup.... and describe and so on

    // TODO: We need to use a stable set of test data.
    // TODO: I.e. we must decide whether the given item is train or test data when we insert it into the database

    const trainSetPercentage = 0.95;

    const datasetFull = prepareCo2ClosingStateTfDataset(states);

    const trainSize = Math.floor(datasetFull.size * trainSetPercentage);

    const batchSize = trainSize; // TODO: Temp

    const trainDataset = datasetFull.take(trainSize).batch(batchSize).prefetch(1);
    const validDataset = datasetFull.skip(trainSize).batch(batchSize).prefetch(1);

    logger.info(`PhPredict: Training set size ${trainSize}. Validation dataset size ${datasetFull.size - trainSize}`);

    const learningRate = 5e-4;
    const optimizer = tf.train.adam(learningRate);

    console.log("Optimizer config: ", optimizer.getConfig());

    var model;

    if (params.retrain) {
        logger.info("Doing retrain");

        model = tf.sequential({
            layers: [
                tf.layers.dense({ units: 40, inputDim: 46, activation: "tanh" }),
                tf.layers.dense({ units: 30, activation: "tanh" }),
                tf.layers.dense({ units: 20, activation: "tanh" }),
                tf.layers.dense({ units: 5, activation: "tanh" }),
                tf.layers.dense({ units: 1, activation: "sigmoid" })
            ]
        });

    } else {
        logger.info("Training existing model");

        model = await loadModelFromFile();
    }

    model.compile({ loss: "meanSquaredError", optimizer });

    await model.fitDataset(trainDataset, { epochs: 200000, verbose: 1, validationData: validDataset });

    console.log("Optimizer config: ", optimizer.getConfig());

    await model.save(minPhPredictionModelSaveLocation);
}

// ---------------------------------------------------------------

async function train() {
    const states = await databaseService.findCo2ClosingStates(Co2ClosingStateType.ANY);

    for (var state of states) {
        const date = new Date(state.closeTime * 1000).toLocaleString("nb");
        console.log(date + ": Offset after close: " + state.minPh600OffsetAfterClose + "  (i.e. " + (state.minPh600OffsetAfterClose + state.ph600AtClose) + ")");
    }

    console.log("Total records: " + states.length);

    await sleep(3000);

    await trainModelFromDataset(states, { retrain: false });

    exit(0);
}

// ========================================================================================

train();
