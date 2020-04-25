import { injectable, postConstruct } from "inversify";

import * as tf from '@tensorflow/tfjs-node'
import slidingWindow from "server/misc/sliding-window";

// API: https://js.tensorflow.org/api/latest/

// TODO: Note: Work with TF must be done in a worker thread to avoid blocking main thread!

@injectable()
export default class Co2PredictionServiceImpl {
    @postConstruct()
    _init() {
    }

    async test() {
        const window_size = 10;

        const data = Array.from({ length: 100 }, (_, i) => i);
        const windows = slidingWindow(data, window_size + 1);

        const inputOutputTensors = windows.map(k => ({
            xs: tf.tensor1d(k.slice(0, -1)),
            ys: tf.tensor1d(k.slice(-1)),
        }));

        const dataset = tf.data.array(inputOutputTensors).shuffle(1000).batch(64).prefetch(1);

        const model = tf.sequential({
            layers: [
                tf.layers.dense({ units: 10, inputDim: window_size, activation: "relu" }),
                tf.layers.dense({ units: 10, activation: "relu" }),
                tf.layers.dense({ units: 1 })
            ]
        });

        model.compile({ loss: "meanSquaredError", optimizer: tf.train.momentum(8e-6, 0.9) });
        await model.fitDataset(dataset, { epochs: 1000, verbose: 1 });

        (model.predict(tf.tensor2d([[210, 211, 212, 213, 214, 215, 216, 217, 218, 219]])) as tf.Tensor).print();
    }
}