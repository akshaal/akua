import * as tf from './tf';
import logger from "server/logger";

import {
    Co2ClosingState,
    Co2ClosingStateOrigin,
    MessageToPhPredictionWorker,
    MinPhPredictionRequest,
    MinPhPredictionResponse
} from '../service/PhPrediction';

import { parentPort } from 'worker_threads';
import { newTimestamp } from 'server/misc/new-timestamp';
import config, { asUrl } from 'server/config';

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
export function createCo2ClosingStateFeaturesAndLabels(state: Co2ClosingState): null | { xs: number[], ys: number[] } {
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

    if (state.ph600OffsetsBeforeClose.length != 21) {
        logger.error("PhPredict: Strange ph600-offset-before-close", { state });
        return null;
    }

    if (state.ph60OffsetsBeforeClose.length != 22) {
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

    // This might affect forecast
    const closedTimeDate = new Date(state.closeTime * 1000);
    const scaledMinutesSinceDayStart = (closedTimeDate.getUTCHours() * 60 + closedTimeDate.getUTCMinutes()) / (60.0 * 24.0);

    var scaledOpenedSecondsAgo = state.openedSecondsAgo / (60 * 60 * 2);
    if (scaledOpenedSecondsAgo > 1.0) {
        scaledOpenedSecondsAgo = 1; // long time ago
    }

    // XS: "Features" or "Input for neural network"
    const xs = [
        ...scaledPh600OffsetBeforeClose,
        ...scaledPh60OffsetBeforeClose,
        scalePh(state.ph600AtClose),
        scaledMinutesSinceDayStart,
        scaledOpenedSecondsAgo
    ];

    // YS: "Labels" or "Output for neural network"
    const ys = [scalePhOffset(state.minPh600OffsetAfterClose)];

    return { xs, ys };
}

// ================================================================

export function loadModelFromFile(): Promise<tf.LayersModel> {
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
    logger.info("PhPredict: Started worker thread for PH predictions");
    parentPort.on('message', onMessageToPhPredictionWorker);
}
