import * as tf from './tf';
import logger from "server/logger";

import {
    Co2ClosingState,
    Co2ClosingStateOrigin,
    MessageToPhPredictionWorker,
    MinPhPredictionRequest,
    MinPhPredictionResponse,
    PH_PREDICTION_WINDOW_LENGTH
} from '../service/PhPrediction';

import { parentPort } from 'worker_threads';
import { newTimestamp } from 'server/misc/new-timestamp';
import config, { asUrl } from 'server/config';
import { calcCo2FromPh } from 'server/misc/calcCo2FromPh';

// TODO: Move to config.... and some other place, not ui!
const minPhPredictionModelLoadLocation = asUrl(config.bindOptions) + "/ui/model.dump";

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
const minPhPredictionModelPromise: Promise<tf.LayersModel> = parentPort ? loadModelFromFile() : new Promise(() => { });

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
 * Rescale OC2 value to a normalized value that is used in TensorFlow network.
 */
function scaleCo2(co2: number): number {
    if (co2 > 50) {
        return 1;
    }

    if (co2 < 0) {
        return 0;
    }

    return co2 / 50.0;
}

/**
 * Rescale temperature value to a normalized value that is used in TensorFlow network.
 */
/*function scaleTemperature(temp: number): number {
    if (temp > 40) {
        return 1;
    }

    if (temp < 0) {
        return 0;
    }

    return temp / 40;
}*/

/**
 * Reverse scalePhOffset operation. Convert from value prepared to NN back to normal PH difference.
 */
function descalePhOffset(scaledDiff: number): number {
    return 0 + scaledDiff * 2 - 1;
}

// ========================================================================================

// We can't extend 'tf.layers.Layer' ;(
export declare interface AkDropTimeseriesLayerArgs {
    /**
     * Number of item to drop on the left (head).
     */
    dropHead?: number;

    /**
     * Number of item to drop on the right (tail).
     */
    dropTail?: number;

    /**
     * Name for the layer.
     */
    name?: string;
}

/**
 * Expects input of shape [m, t, f] and outputs shape [m, t - dropHead - dropTail, f].
 * Where m are examples, t are timeseries and f are feature in timeseries.
 */
export class AkDropTimeseriesLayer extends tf.layers.Layer {
    static className = 'AkDropTimeseriesLayer';

    private _dropHead = this._args.dropHead || 0;
    private _dropTail = this._args.dropTail || 0;

    constructor(private _args: AkDropTimeseriesLayerArgs) {
        super(_args);
    }

    computeOutputShape(inputShape: tf.Shape): tf.Shape {
        // TODO: Check shape!

        const outputShape = [
            inputShape[0],
            (inputShape[1] as number) - this._dropHead - this._dropTail,
            inputShape[2]
        ];

        return outputShape;
    }

    getConfig(): tf.serialization.ConfigDict {
        const config = { dropHead: this._dropHead, dropTail: this._dropTail };
        const baseConfig = super.getConfig();
        Object.assign(config, baseConfig);
        return config;
    }

    call(inputs: tf.Tensor | tf.Tensor[]): tf.Tensor {
        return tf.tidy(() => {
            if (Array.isArray(inputs)) {
                if (inputs.length !== 1) {
                    throw new Error(`AkDropTimeseriesLayer.call expected Tensor length to be 1; got ${inputs.length}`);
                }
                inputs = inputs[0];
            } else {
                inputs = inputs;
            }

            return tf.slice(inputs, [0, this._dropHead, 0], [-1, (inputs.shape[1] as number) - this._dropTail - this._dropHead, -1]);
        });
    }
}

tf.serialization.registerClass(AkDropTimeseriesLayer);

export function akDropTimeseriesLayer(args: AkDropTimeseriesLayerArgs): AkDropTimeseriesLayer {
    return new AkDropTimeseriesLayer(args);
}

// ========================================================================================

/**
 * Translates state into features (inputs) and labels (outputs)
 * returns null if data is invalid
 */
export function createCo2ClosingStateFeaturesAndLabels(state: Co2ClosingState): null | { xs: number[][], ys: number[] } {
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

    if (state.ph60Offset15sInterval.length != PH_PREDICTION_WINDOW_LENGTH) {
        logger.error("PhPredict: Strange ph60-array", { state });
        return null;
    }

    if (state.temperature15sInterval.length != PH_PREDICTION_WINDOW_LENGTH) {
        logger.error("PhPredict: Strange temperature array", { state });
        return null;
    }

    if (state.dayLightOn15sInterval.length != PH_PREDICTION_WINDOW_LENGTH) {
        logger.error("PhPredict: Strange day light array", { state });
        return null;
    }

    if (state.co2ValveOpen15sInterval.length != PH_PREDICTION_WINDOW_LENGTH) {
        logger.error("PhPredict: Strange co2 valve open array", { state });
        return null;
    }

    // -----------------------------------------------------------------

    const xs: number[][] = [];

    for (var i = 0; i < PH_PREDICTION_WINDOW_LENGTH; i++) {
        const ph60Offset = state.ph60Offset15sInterval[i];
        if (ph60Offset < -4 || ph60Offset > 4 || typeof ph60Offset !== "number") {
            logger.error("PhPredict: Strange value in ph60-offset-before-close", { state });
            return null;
        }

        const temp = state.temperature15sInterval[i];
        if (temp < 10 || temp > 40 || typeof temp !== "number") {
            logger.error("PhPredict: Strange value in temperatures array", { state });
            return null;
        }

        const co2ValveOpen = state.co2ValveOpen15sInterval[i];
        if (typeof co2ValveOpen !== "boolean") {
            logger.error("PhPredict: Strange value in co2ValveOpen array", { state });
            return null;
        }

        const dayLightOn = state.dayLightOn15sInterval[i];
        if (typeof dayLightOn !== "boolean") {
            logger.error("PhPredict: Strange value in dayLightOn array", { state });
            return null;
        }

        xs.push([
            scalePh(state.ph600AtClose),
            scalePhOffset(ph60Offset),
            scaleCo2(calcCo2FromPh(config.aquaEnv, state.ph600AtClose + ph60Offset)),
            //scaleTemperature(temp), to avoid over-fitting
            dayLightOn ? 1 : 0,
            co2ValveOpen ? 1 : 0,
        ]);
    }

    // YS: "Labels" or "Output for neural network"
    const ys = [scalePhOffset(state.minPh600OffsetAfterClose)];

    return { xs, ys };
}

// ================================================================

function loadModelFromFile(): Promise<tf.LayersModel> {
    logger.info("PhPredict: Loading min-PH prediction model from " + minPhPredictionModelLoadLocation);
    return tf.loadLayersModel(minPhPredictionModelLoadLocation + "/model.json");
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
        const tfPrediction = model.predict(tf.tensor3d([featuresAndLabels.xs])) as tf.Tensor;

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
    logger.info("PhPredict: Started worker thread for PH predictions");
    parentPort.on('message', onMessageToPhPredictionWorker);
}
