import "reflect-metadata";
import logger from "server/logger";
import config from "server/config";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState, createCo2ClosingState, Co2ClosingStateOrigin, PH_PREDICTION_WINDOW_LENGTH } from "server/service/PhPrediction";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import * as tf from 'server/service_impl/tf';
import { createCo2ClosingStateFeaturesAndLabels } from "server/service_impl/PhPredictionWorkerThread";
import { writeFileSync } from "fs";
import { readFileSync } from "fs";
import { isPresent } from "server/misc/isPresent";

// TODO: Need comments and cleanup!

// TODO: Use config!
const minPhPredictionModelLocationForCli = 'file://server/static-ui/model.dump';

type Co2ClosingStateTfData = { xs: tf.Tensor2D, ys: tf.Tensor1D };
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

/*function sleep(ms: number) {
    return new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}*/

// ========================================================================================

// TODO: Remove
export function prepareData(): Co2ClosingState[] {
    const result: Co2ClosingState[] = [];

    logger.info("Preparing data from json files!");

    const ph600Map: { [k: number]: number } = {};
    const ph60Map: { [k: number]: number } = {};
    const openMap: { [k: number]: number } = {};
    const lightOnMap: { [k: number]: number } = {};
    const tempMap: { [k: number]: number } = {};
    const keys: number[] = [];

    const ph600sJson = JSON.parse(readFileSync("server/static-ui/ph600.json").toString("UTF-8"));
    const ph60sJson = JSON.parse(readFileSync("server/static-ui/ph60.json").toString("UTF-8"));
    const openJson = JSON.parse(readFileSync("server/static-ui/open.json").toString("UTF-8"));
    const lightOnJson = JSON.parse(readFileSync("server/static-ui/light.json").toString("UTF-8"));
    const tempJson = JSON.parse(readFileSync("server/static-ui/temp.json").toString("UTF-8"));

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

    lightOnJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        lightOnMap[k] = parseFloat(v[1]);
    });

    console.log("Time points in light-map:", Object.keys(lightOnMap).length);

    tempJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        tempMap[k] = parseFloat(v[1]);
    });

    console.log("Time points in temp-map:", Object.keys(tempMap).length);

    for (const k of [...keys]) {
        if (ph600Map[k] === undefined || ph60Map[k] === undefined || openMap[k] === undefined || tempMap[k] === undefined || lightOnMap[k] === undefined) {
            const kI = keys.indexOf(k);
            console.log("deleted key", kI);
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
                    closeTime: valveCloseK,
                    openedSecondsAgo: valveOpenedSecondsBeforeClose,
                    minPh600: minPh600,
                    origin: Co2ClosingStateOrigin.ThisInstance,
                    getPh600: (t: number) => ph600Map[t],
                    getPh60: (t: number) => ph60Map[t],
                    getTemperature: (t: number) => tempMap[t],
                    isDayLightOn: (t: number) => isPresent(lightOnMap[t]) ? (lightOnMap[t] == 1) : undefined,
                    isCo2ValveOpen: (t: number) => isPresent(openMap[t]) ? (openMap[t] == 1) : undefined,
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

/*
export async function prepareDataAsync(): Promise<void> {
    const result = prepareData();

    const existingStates = await databaseService.findCo2ClosingStates(Co2ClosingStateType.ANY);
    const existingTsMap: {[k: number]: true} = {};
    for (var state of existingStates) {
        existingTsMap[state.closeTime] = true;
    }

    for (var item of result) {
        if (!existingTsMap[item.closeTime]) {
            await databaseService.insertCo2ClosingState(item);
            console.log("Added new item: ", item);
        }
    }

    logger.info("Done inserting new records! count=" + result.length);

    exit(0);
}
*/

// ---------------------------------------------------------------

function prepareCo2ClosingStateTfDataset(states: Readonly<Co2ClosingState[]>): Co2ClosingStateTfDataset {
    const dataArray: Co2ClosingStateTfData[] = [];

    for (const state of states) {
        const featuresAndLabels = createCo2ClosingStateFeaturesAndLabels(state);
        if (!featuresAndLabels) {
            continue;
        }

        dataArray.push({
            xs: tf.tensor2d(featuresAndLabels.xs),
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
    const trainBatchSize = params.trainingStates.length;

    const trainDataset = prepareCo2ClosingStateTfDataset(params.trainingStates).batch(trainBatchSize).prefetch(1);
    const validDataset = prepareCo2ClosingStateTfDataset(params.validationStates).batch(params.validationStates.length).prefetch(1);

    //const learningRate = undefined;
    //const learningRate = 5e-4;
    //const learningRate = 1e-5;
    //const learningRate = 5e-6;
    const learningRate = 5e-5;

    const optimizer = tf.train.adam(learningRate);

    console.log("Optimizer config: ", optimizer.getConfig());

    var model;

    if (params.retrain) {
        logger.info("Doing retrain");

        model = tf.sequential({
            layers: [
                tf.layers.conv1d({
                    kernelSize: 4,
                    filters: 3,
                    activation: "selu",
                    kernelInitializer: 'leCunNormal',
                    inputShape: [PH_PREDICTION_WINDOW_LENGTH, 5],
                    strides: 2
                }),
                tf.layers.conv1d({
                    kernelSize: 4,
                    filters: 6,
                    activation: "selu",
                    kernelInitializer: 'leCunNormal',
                    strides: 2
                }),
                tf.layers.gru({
                    units: 4,
                    activation: "selu",
                    kernelInitializer: 'leCunNormal'
                }),
                tf.layers.dense({ units: 3, activation: "selu", kernelInitializer: 'leCunNormal' }),
                tf.layers.dense({ units: 1, activation: "selu", kernelInitializer: 'leCunNormal' })
            ]
        });

    } else {
        logger.info("Training existing model");
        model = await tf.loadLayersModel(minPhPredictionModelLocationForCli + "/model.json");
    }

    model.compile({ loss: "meanSquaredError", optimizer });
    model.summary();

    /*
    better weight dumping code...
        for (var w of model.getWeights()) {
        console.log("------------------------");
        console.log(w);
        console.log(w.arraySync());
    }
    */

    const result = await model.fitDataset(trainDataset, { epochs: 20_000, verbose: 1, validationData: validDataset });

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

    //await sleep(3000);

    await trainModelFromDataset({
        trainingStates,
        validationStates,
        retrain: false
    });

    exit(0);
}

// ========================================================================================

train();

