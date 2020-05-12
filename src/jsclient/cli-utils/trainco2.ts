import "reflect-metadata";
import logger from "server/logger";
import config from "server/config";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState, createCo2ClosingState, Co2ClosingStateOrigin } from "server/service/PhPrediction";
import DatabaseService from "server/service/DatabaseService";
import * as tf from 'server/service_impl/tf';
import { createCo2ClosingStateFeaturesAndLabels, loadModelFromFile } from "server/service_impl/PhPredictionWorkerThread";
import { readFileSync } from "fs";

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

// ========================================================================================

// TODO: Remove
export function prepareData(): Co2ClosingState[] {
    const result: Co2ClosingState[] = [];

    logger.info("Preparing data from json files!");

    const ph600Map: { [k: number]: number } = {};
    const ph60Map: { [k: number]: number } = {};
    const openMap: { [k: number]: number } = {};
    const keys: number[] = [];

    const ph600sJson = JSON.parse(readFileSync("server/static-ui/ph600s.json").toString("UTF-8"));
    const ph60sJson = JSON.parse(readFileSync("server/static-ui/ph60s.json").toString("UTF-8"));
    const openJson = JSON.parse(readFileSync("server/static-ui/open.json").toString("UTF-8"));

    ph600sJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        ph600Map[k] = parseFloat(v[1]);
        keys.push(k);
    });

    console.log("Time points in ph600-map:", Object.keys(ph600Map).length);

    ph60sJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        ph60Map[k] = parseFloat(v[1]);
    });

    console.log("Time points in ph60-map:", Object.keys(ph60Map).length);

    openJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        openMap[k] = parseFloat(v[1]);
    });

    console.log("Time points in open-map:", Object.keys(openMap).length);

    for (const k of [...keys]) {
        if (ph600Map[k] === undefined || ph60Map[k] === undefined || openMap[k] === undefined) {
            const kI = keys.indexOf(k);
            console.log(kI);
            keys.splice(kI, 1);
        }
    }

    console.log("Common time points:", keys.length);

    // ---------------------------------------------------------------------------
    // Find points of valve turn-off

    var prevValveOpen = openMap[keys[0]];
    var valveCloseK: number = 0;
    var valveOpenedSecondsBeforeClose: number = 0;
    var valveOpenK: number = 0;
    var minPh600: number = 0;

    for (const k of keys) {
        const valveOpen = openMap[k];

        if (valveOpen === 0) {
            // Valve is closed, find the lowest value for ph600
            if (minPh600 > ph600Map[k]) {
                minPh600 = ph600Map[k];
            }
        }

        if (valveOpen === prevValveOpen) {
            // No changes in valve state
            continue;
        }

        prevValveOpen = valveOpen;

        if (valveOpen === 0) {
            // Valve is just closed
            valveCloseK = k;
            minPh600 = ph600Map[k];
            valveOpenedSecondsBeforeClose = k - valveOpenK;
            valveOpenK = 0;
        } else {
            valveOpenK = k;

            // Valve is just opened
            if (valveCloseK) {
                if (k - valveCloseK < 600) {
                    console.log("Too short closed interval", k - valveCloseK);
                    continue;
                }

                if (k - valveCloseK > (6 * 60 * 60)) {
                    console.log("Too long closed interval", (k - valveCloseK) / 60 / 60, "hours @ ", new Date(k * 1000).getHours(), "o'clock");
                    continue;
                }

                if (valveOpenedSecondsBeforeClose < 0) {
                    console.log("Negative open interval", valveOpenedSecondsBeforeClose);
                    continue;
                }

                if (valveOpenedSecondsBeforeClose > (60 * 60 * 12)) {
                    console.log("Unreal open interval", valveOpenedSecondsBeforeClose);
                    continue;
                }

                const state = createCo2ClosingState({
                    tClose: valveCloseK,
                    openedSecondsAgo: valveOpenedSecondsBeforeClose,
                    minPh600: minPh600,
                    origin: Co2ClosingStateOrigin.ThisInstance,
                    getPh600: (t: number) => ph600Map[t],
                    getPh60: (t: number) => ph60Map[t],
                });

                if (state) {
                    result.push(state);
                } else {
                    console.log("Bad data in interval");
                }
            }

            valveCloseK = 0;
        }
    }

    logger.info("count=" + result.length);

    return result;
}

export async function prepareDataAsync(): Promise<void> {
    const result = prepareData();

    for (var item of result) {
        await databaseService.insertCo2ClosingState(item);
    }

    logger.info("Done inserting new records! count=" + result.length);

    exit(0);
}

// ========================================================================================

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

    const trainSetPercentage = 0.95;

    const datasetFull = prepareCo2ClosingStateTfDataset(states);

    const trainSize = Math.floor(datasetFull.size * trainSetPercentage);

    const batchSize = trainSize; // TODO: Temp

    const trainDataset = datasetFull.take(trainSize).batch(batchSize).prefetch(1);
    const validDataset = datasetFull.skip(trainSize).batch(batchSize).prefetch(1);

    logger.info(`PhPredict: Training set size ${trainSize}. Validation dataset size ${datasetFull.size - trainSize}`);

    const optimizer = tf.train.adam();

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

    await model.fitDataset(trainDataset, { epochs: 1200000, verbose: 1, validationData: validDataset });

    await model.save(minPhPredictionModelSaveLocation);
}

// ---------------------------------------------------------------

async function train() {
    const states = await databaseService.findCo2ClosingStates();

    console.log(states);
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

// prepareDataAsync();

train();