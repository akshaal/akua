import { injectable, postConstruct } from "inversify";

import * as tf from '@tensorflow/tfjs-node'
import slidingWindow from "server/misc/sliding-window";
import { writeFileSync } from "fs";

// API: https://js.tensorflow.org/api/latest/

// TODO: Note: Work with TF must be done in a worker thread to avoid blocking main thread!

enum TrainingSampleVersion {
    V0 = 0
};

enum TrainingSampleOrigin {
    ThisInstance = 0,
    OtherInstance = 1
};

interface TrainingSample {
    origin: TrainingSampleOrigin,
    version: TrainingSampleVersion,

    // State at the moment of close operation
    closeTime: number;
    ph600AtClose: number;
    ph600OffsetsBeforeClose: number[];
    ph60OffsetsBeforeClose: number[];

    // Close-action output (result of close operation)
    minPh600OffsetAfterClose: number;
};

@injectable()
export default class Co2PredictionServiceImpl {
    @postConstruct()
    _init() {
    }

    prepareData() {
        const result: TrainingSample[] = [];

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
                    var bad = false;

                    // norm between 6 and 8
                    const ph600AtClose = ph600Map[valveCloseK];

                    const histPh600 = [];
                    const histPh60 = [];

                    for (var m = 1; m < 10; m += 1) {
                        const ph600AtM = ph600Map[valveCloseK - 60 * m];
                        const ph60AtM = ph60Map[valveCloseK - 60 * m];

                        histPh600.unshift(ph600AtM - ph600AtClose);
                        histPh60.unshift(ph60AtM - ph600AtClose);

                        bad = bad || (ph600AtM === undefined || ph600AtM < 5 || ph600AtM > 9);
                        bad = bad || (ph60AtM === undefined || ph60AtM < 5 || ph60AtM > 9);
                    }

                    if (bad) {
                        console.log("Bad data in interval");
                    } else {
                        result.push({
                            origin: TrainingSampleOrigin.ThisInstance,
                            version: TrainingSampleVersion.V0,

                            // State at the moment of close operation
                            closeTime: valveCloseK,
                            ph600AtClose: ph600AtClose,
                            ph600OffsetsBeforeClose: histPh600,
                            ph60OffsetsBeforeClose: histPh60,
                        
                            // Close-action output (result of close operation)
                            minPh600OffsetAfterClose: minPh600 - ph600AtClose
                        });
                    }
                }

                valveCloseK = 0;
            }

            prevValveOpen = valveOpen;
        }

        writeFileSync("server/static-ui/training-set.json", JSON.stringify(result));
    }

    // ---------------------------------------------------------------

    async test() {
        const window_size = 10;

        const data = Array.from({ length: 200 }, (_, i) => i);
        const windows = slidingWindow(data, window_size + 1);

        const inputOutputTensors = windows.map(k => ({
            xs: tf.tensor1d(k.slice(0, -1)),
            ys: tf.tensor1d(k.slice(-1)),
        }));

        const datasetFull = tf.data.array(inputOutputTensors).shuffle(1000);
        const dataset = datasetFull.take(100).batch(64).prefetch(1);
        const validDataset = datasetFull.skip(100).batch(64).prefetch(1);

        const model = tf.sequential({
            layers: [
                tf.layers.dense({ units: 10, inputDim: window_size, activation: "relu" }),
                tf.layers.dense({ units: 10, activation: "relu" }),
                tf.layers.dense({ units: 1 })
            ]
        });

        model.compile({ loss: "meanSquaredError", optimizer: tf.train.momentum(8e-6, 0.9) });
        await model.fitDataset(dataset, { epochs: 10000, verbose: 1, validationData: validDataset });

        (model.predict(tf.tensor2d([[210, 211, 212, 213, 214, 215, 216, 217, 218, 219]])) as tf.Tensor).print();
    }
}