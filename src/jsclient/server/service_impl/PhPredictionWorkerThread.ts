import * as tf from '@tensorflow/tfjs'
import { writeFileSync, readFileSync } from "fs";
import logger from "server/logger";

import {
    Co2ClosingState,
    Co2ClosingStateOrigin,
    createCo2ClosingState,
    MessageToPhPredictionWorker,
    MinPhPredictionRequest,
    MinPhPredictionResponse
} from './PhPrediction';

import { parentPort } from 'worker_threads';
import { newTimestamp } from 'server/misc/new-timestamp';
import config, { asUrl } from 'server/config';

// TODO: Move to config.
const minPhPredictionModelLocation = asUrl(config.bindOptions) + "/ui/model.dump";

// Log startup event if we are in a worker thread
if (parentPort) {
    logger.info("PhPredict: Started worker thread for PH predictions");
}

// TODO: !!! Export training set to be able to draw it and see what NN
// TODO: !!! actually see and decide what info it needs to import quality of prediction

// TODO: Better names
// TODO: Review exported functions (some of them useful in tests)
// TODO: Statistics

// API: https://js.tensorflow.org/api/latest/

/**
 * Promise of currently used min-ph prediction model.
 * Initial promise is not resolved.
 * In tests we need to call loadModelFromFIle to load the model from file obviously.
 * If this file is opened as a worker thread, then the call is done automatically.
 */
var minPhPredictionModelPromise: Promise<tf.LayersModel> = new Promise(() => { });

type Co2ClosingStateTfData = { xs: tf.Tensor1D, ys: tf.Tensor1D };
type Co2ClosingStateTfDataset = tf.data.Dataset<Co2ClosingStateTfData>;

/**
 * Rescale PH value to a normalized value that is used in TensorFlow network.
 */
function scalePh(ph: number): number {
    if (ph > 8) {
        return 1;
    }

    if (ph < 6) {
        return 0;
    }

    return (ph - 6) / 2.0;
}

/**
 * Rescale PH difference to a normalized value that is used in TensorFlow network.
 */
function scalePhOffset(diff: number): number {
    const rescaled = (diff + 1) / 2;

    if (rescaled < 0) {
        return 0;
    }

    if (rescaled > 1) {
        return 1;
    }

    return rescaled;
}

/**
 * Reverse scalePhOffset operation. Convert from value prepared to NN back to normal PH difference.
 */
function descalePhOffset(scaledDiff: number): number {
    return 0 + scaledDiff * 2 - 1;
}

// ========================================================================================

/**
 * Translates state into features (inputs) and labels (outputs)
 * returns null if data is invalid
 */
function createCo2ClosingStateFeaturesAndLabels(state: Co2ClosingState): null | { xs: number[], ys: number[] } {
    // Validate state data

    if (!state.closeTime) {
        logger.error("PhPredict: Missing close-time in co2-closing-dataset", { state });
        return null;
    }

    if (state.origin !== Co2ClosingStateOrigin.OtherInstance && state.origin !== Co2ClosingStateOrigin.ThisInstance) {
        logger.error("PhPredict: Unknown origin in co2-closing-dataset", { state });
        return null;
    }

    if (state.minPh600OffsetAfterClose > 4 || state.minPh600OffsetAfterClose < -4 || typeof state.minPh600OffsetAfterClose != "number") {
        logger.error("PhPredict: Strange min-ph-600-offset-after-close in co2-closing-dataset", { state });
        return null;
    }

    if (state.ph600AtClose > 8 || state.ph600AtClose < 4) {
        logger.error("PhPredict: Strange ph-600-at-close", { state });
        return null;
    }

    if (state.ph600OffsetsBeforeClose.length != 18) {
        logger.error("PhPredict: Strange ph600-offset-before-close", { state });
        return null;
    }

    if (state.ph60OffsetsBeforeClose.length != 19) {
        logger.error("PhPredict: Strange ph60-offset-before-close", { state });
        return null;
    }

    for (var i = 0; i < 18; i++) {
        const ph600Offset = state.ph600OffsetsBeforeClose[i];
        if (ph600Offset < -4 || ph600Offset > 4 || typeof ph600Offset != "number") {
            logger.error("PhPredict: Strange value in ph600-offset-before-close", { state });
            return null;
        }
    }

    for (var i = 0; i < 19; i++) {
        const ph60Offset = state.ph60OffsetsBeforeClose[i];
        if (ph60Offset < -4 || ph60Offset > 4 || typeof ph60Offset != "number") {
            logger.error("PhPredict: Strange value in ph60-offset-before-close", { state });
            return null;
        }
    }

    // Add validated state into data array that will be used to create dataset for tensorflow
    const scaledPh600OffsetBeforeClose = state.ph600OffsetsBeforeClose.map(scalePhOffset);
    const scaledPh60OffsetBeforeClose = state.ph60OffsetsBeforeClose.map(scalePhOffset);

    // XS: "Features" or "Input for neural network"
    const xs = [
        ...scaledPh600OffsetBeforeClose,
        ...scaledPh60OffsetBeforeClose,
        scalePh(state.ph600AtClose)
    ];

    // YS: "Labels" or "Output for neural network"
    const ys = [scalePhOffset(state.minPh600OffsetAfterClose)];

    return { xs, ys };
}

// ========================================================================================

// TODO: Review, give better name
export function prepareData() {
    const result: Co2ClosingState[] = [];

    const ph600Map: { [k: number]: number } = {};
    const ph60Map: { [k: number]: number } = {};
    const openMap: { [k: number]: number } = {};
    const keys: number[] = [];

    const ph600sJson = require("server/static-ui/ph600s.json");
    const ph60sJson = require("server/static-ui/ph60s.json");
    const openJson = require("server/static-ui/open.json");

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

        if (valveOpen === 0) {
            // Valve is just closed
            valveCloseK = k;
            minPh600 = ph600Map[k];
        } else {
            // Valve is just opened
            if (valveCloseK) {
                if (k - valveCloseK < 600) {
                    console.log("Too short interval");
                    continue;
                }

                const state = createCo2ClosingState({
                    tClose: valveCloseK,
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

        prevValveOpen = valveOpen;
    }

    writeFileSync("server/static-ui/training-set.json", JSON.stringify(result));
}

// =======================================================================================

// TODO: This is for test only.....
export async function testModel() {
    const model = await minPhPredictionModelPromise;

    const result: { x: number, y: number, group: 3 }[] = [];

    const ph600Map: { [k: number]: number } = {};
    const ph60Map: { [k: number]: number } = {};
    const openMap: { [k: number]: number } = {};
    const keys: number[] = [];

    const ph600sJson = require("server/static-ui/ph600s.json");
    const ph60sJson = require("server/static-ui/ph60s.json");
    const openJson = require("server/static-ui/open.json");

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

    for (const k of keys) {
        //if (result.length > 100) {
        //    break;
        //}

        const valveOpen = openMap[k];

        if (valveOpen === 0) {
            continue;
        }

        const state = createCo2ClosingState({
            tClose: k,
            minPh600: ph600Map[k],
            origin: Co2ClosingStateOrigin.ThisInstance,
            getPh600: (t: number) => ph600Map[t],
            getPh60: (t: number) => ph60Map[t],
        });

        if (!state) {
            console.log("Bad data in interval");
        } else {
            const featuresAndLabels = createCo2ClosingStateFeaturesAndLabels(state);
            if (!featuresAndLabels) {
                continue;
            }

            // TODO: Get statistics and stuff
            const predicatedScaledDiff = (((model.predict(tf.tensor2d([featuresAndLabels.xs])) as tf.Tensor).arraySync()) as any)[0][0];
            const predictedPh = ph600Map[k] + predicatedScaledDiff * 2 - 1;

            result.push({ x: k, y: predictedPh, group: 3 });
        }
    }

    writeFileSync("server/static-ui/predictions.json", JSON.stringify(result));
}

// ---------------------------------------------------------------

function prepareCo2ClosingStateTfDataset(): Co2ClosingStateTfDataset {
    const stateCo2ClosingDatasetJson: Co2ClosingState[] = JSON.parse(readFileSync("server/static-ui/training-set.json").toString("UTF-8"));

    const dataArray: Co2ClosingStateTfData[] = [];

    for (const state of stateCo2ClosingDatasetJson) {
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

export async function retrainModelFromDataset() {
    // Train

    // TODO: Cleanup.... and describe and so on

    const datasetFull = prepareCo2ClosingStateTfDataset();
    const trainDataset = datasetFull.take(120).batch(4).prefetch(1);
    const validDataset = datasetFull.skip(120).batch(4).prefetch(1);

    const model = tf.sequential({
        layers: [
            tf.layers.dense({ units: 10, inputDim: 38, activation: "tanh" }),
            tf.layers.dense({ units: 10, activation: "tanh" }),
            tf.layers.dense({ units: 1 })
        ]
    });

    model.compile({ loss: "meanAbsoluteError", optimizer: tf.train.momentum(8e-6, 0.9) });
    await model.fitDataset(trainDataset, { epochs: 10000, verbose: 1, validationData: validDataset });

    await model.save(minPhPredictionModelLocation);

    minPhPredictionModelPromise = Promise.resolve(model);
}

// ================================================================

export function loadModelFromFile() {
    minPhPredictionModelPromise = tf.loadLayersModel(minPhPredictionModelLocation + "/model.json");
    logger.info("PhPredict: Loading min-PH prediction model from " + minPhPredictionModelLocation);
}

if (parentPort) {
    loadModelFromFile();
}

// ================================================================

/**
 * Called from onMessageToPhPredictionWorker to handle this kind of messages.
 * Must predict PH and send the PhPredictionResponse o main thread via using parentPort object.
 * 
 * @param request predication request
 */
function onPhPredictionRequest(request: MinPhPredictionRequest) {
    const requestTimestamp = newTimestamp();

    const featuresAndLabels = createCo2ClosingStateFeaturesAndLabels(request.co2ClosingState);
    if (!featuresAndLabels) {
        logger.error("PhPredict: Wrong request co2closingSTate", { request });
        return;
    }

    minPhPredictionModelPromise.then(model => {
        const tfPrediction = model.predict(tf.tensor2d([featuresAndLabels.xs])) as tf.Tensor;

        tfPrediction.array().then((tfPredictionArray: any) => {
            const predicatedScaledDiff = tfPredictionArray[0][0];

            const predictedMinPh = request.co2ClosingState.ph600AtClose + descalePhOffset(predicatedScaledDiff);

            const response: MinPhPredictionResponse = {
                type: 'min-ph-prediction-response',
                minPhPrediction: predictedMinPh,
                requestTimestamp
            };

            parentPort?.postMessage(response);
        });
    });
}

// ================================================================

export function onMessageToPhPredictionWorker(message: MessageToPhPredictionWorker) {
    if (message.type === 'min-ph-prediction-request') {
        onPhPredictionRequest(message);
    } else {
        logger.error("PhPredict: Unknown message: ", { message });
    }
}

// ================================================================
// ================================================================

// This class might be also opened from a test (spec), where parentPort is not available
if (parentPort) {
    parentPort.on('message', onMessageToPhPredictionWorker);
}
