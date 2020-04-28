import { injectable, postConstruct } from "inversify";

import * as tf from '@tensorflow/tfjs-node'
import { writeFileSync, readFileSync } from "fs";
import logger from "server/logger";

// API: https://js.tensorflow.org/api/latest/

// TODO: Note: Work with TF must be done in a worker thread to avoid blocking main thread!

type Co2ClosingStateTfData = { xs: tf.Tensor1D, ys: tf.Tensor1D };
type Co2ClosingStateTfDataset = tf.data.Dataset<Co2ClosingStateTfData>;

/**
 * Origin of the data sample. We can initially train using data from other instance
 * and then gradually replace them by data from this instance..
 */
enum Co2ClosingStateOrigin {
    ThisInstance = 0,
    OtherInstance = 1
};

/**
 * Information about state at the moment of CO-closing event.
 * It contains information about PH related values right before
 * the moment when CO2-valve was closed as well as the minimum
 * reached PH between the close-event and the moment when CO2-valve 
 * was opened again.
 */
interface Co2ClosingState {
    origin: Co2ClosingStateOrigin,

    // State at the moment of close operation
    closeTime: number;
    ph600AtClose: number;
    ph600OffsetsBeforeClose: number[];
    ph60OffsetsBeforeClose: number[];

    // Close-action output (result of close operation)
    minPh600OffsetAfterClose: number;
};

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

// ========================================================================================

/**
 * Translates state into features (inputs) and labels (outputs)
 * returns null if data is invalid
 */
function createCo2ClosingStateFeaturesAndLabels(state: Co2ClosingState): null | { xs: number[], ys: number[] } {
    // Validate state data

    if (!state.closeTime) {
        logger.error("Co2Predict: Missing close-time in co2-closing-dataset", { state });
        return null;
    }

    if (state.origin !== Co2ClosingStateOrigin.OtherInstance && state.origin !== Co2ClosingStateOrigin.ThisInstance) {
        logger.error("Co2Predict: Unknown origin in co2-closing-dataset", { state });
        return null;
    }

    if (state.minPh600OffsetAfterClose > 4 || state.minPh600OffsetAfterClose < -4 || typeof state.minPh600OffsetAfterClose != "number") {
        logger.error("Co2Predict: Strange min-ph-600-offset-after-close in co2-closing-dataset", { state });
        return null;
    }

    if (state.ph600AtClose > 8 || state.ph600AtClose < 4) {
        logger.error("Co2Predict: Strange ph-600-at-close", { state });
        return null;
    }

    if (state.ph600OffsetsBeforeClose.length != 18) {
        logger.error("Co2Predict: Strange ph600-offset-before-close", { state });
        return null;
    }

    if (state.ph60OffsetsBeforeClose.length != 19) {
        logger.error("Co2Predict: Strange ph60-offset-before-close", { state });
        return null;
    }

    for (var i = 0; i < 18; i++) {
        const ph600Offset = state.ph600OffsetsBeforeClose[i];
        if (ph600Offset < -4 || ph600Offset > 4 || typeof ph600Offset != "number") {
            logger.error("Co2Predict: Strange value in ph600-offset-before-close", { state });
            return null;
        }
    }

    for (var i = 0; i < 19; i++) {
        const ph60Offset = state.ph60OffsetsBeforeClose[i];
        if (ph60Offset < -4 || ph60Offset > 4 || typeof ph60Offset != "number") {
            logger.error("Co2Predict: Strange value in ph60-offset-before-close", { state });
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

/**
 * Attempts to create Co2ClosingState for the time t.
 */
function createCo2ClosingState(params: {
    tClose: number,
    minPh600: number,
    origin: Co2ClosingStateOrigin,
    getPh600(t: number): number,
    getPh60(t: number): number,
}): Co2ClosingState | null {
    const { tClose, getPh600, getPh60, origin, minPh600 } = params;

    const ph600AtTClose = getPh600(tClose);

    const histPh600 = [];
    const histPh60 = [];

    // Use Ph60 values: 0 seconds ago, 15, 30 and 45 seconds ago
    // Use Ph600 values: 15, 30 and 45 seconds ago
    // (we don't use ph600 0 seconds ago because offset will be zero from the ph600AtTClose)
    for (var s = 0; s < 4; s += 1) {
        const ph60AtM = getPh60(tClose - 15 * s);
        histPh60.unshift(ph60AtM - ph600AtTClose);

        if (ph60AtM === undefined || ph60AtM < 5 || ph60AtM > 9) {
            return null;
        }

        if (s !== 0) {
            const ph600AtM = getPh600(tClose - 15 * s);
            histPh600.unshift(ph600AtM - ph600AtTClose);

            if (ph600AtM === undefined || ph600AtM < 5 || ph600AtM > 9) {
                return null;
            }
        }
    }

    // Use Ph60/Ph600 values: every minutes for 15 minutes back (15 values)
    for (var m = 1; m < 16; m += 1) {
        const ph600AtM = getPh600(tClose - 60 * m);
        const ph60AtM = getPh60(tClose - 60 * m);

        histPh600.unshift(ph600AtM - ph600AtTClose);
        histPh60.unshift(ph60AtM - ph600AtTClose);

        if (ph600AtM === undefined || ph600AtM < 5 || ph600AtM > 9) {
            return null;
        }

        if (ph60AtM === undefined || ph60AtM < 5 || ph60AtM > 9) {
            return null;
        }
    }

    return {
        origin,
        closeTime: tClose,
        ph600AtClose: ph600AtTClose,
        ph600OffsetsBeforeClose: histPh600,
        ph60OffsetsBeforeClose: histPh60,
        minPh600OffsetAfterClose: minPh600 - ph600AtTClose
    };
}

// ========================================================================================

@injectable()
export default class Co2PredictionServiceImpl {
    @postConstruct()
    _init() {
    }

    prepareData() {
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

    testModel(model: tf.LayersModel) {
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

                // TODO: Sync? Get statistics and stuff
                const predicatedScaledDiff = (((model.predict(tf.tensor2d([featuresAndLabels.xs])) as tf.Tensor).arraySync()) as any)[0][0];
                const predictedPh = ph600Map[k] + predicatedScaledDiff * 2 - 1;

                result.push({ x: k, y: predictedPh, group: 3 });
            }
        }

        writeFileSync("server/static-ui/predictions.json", JSON.stringify(result));
    }

    // ---------------------------------------------------------------

    prepareCo2ClosingStateTfDataset(): Co2ClosingStateTfDataset {
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

    async test() {
        const modelLocation = 'file://server/static-ui/model.dump';
        const load = true;
        var model: tf.LayersModel;

        if (load) {
            // Load existing
            model = await tf.loadLayersModel(modelLocation + "/model.json");
        } else {
            // Train

            const datasetFull = this.prepareCo2ClosingStateTfDataset();
            const trainDataset = datasetFull.take(108).batch(4).prefetch(1);
            const validDataset = datasetFull.skip(108).batch(4).prefetch(1);

            model = tf.sequential({
                layers: [
                    tf.layers.dense({ units: 10, inputDim: 38, activation: "tanh" }),
                    tf.layers.dense({ units: 10, activation: "tanh" }),
                    tf.layers.dense({ units: 1 })
                ]
            });

            model.compile({ loss: "meanAbsoluteError", optimizer: tf.train.momentum(8e-6, 0.9) });
            await model.fitDataset(trainDataset, { epochs: 10000, verbose: 1, validationData: validDataset });

            await model.save(modelLocation);
        }

        this.testModel(model);
    }
}