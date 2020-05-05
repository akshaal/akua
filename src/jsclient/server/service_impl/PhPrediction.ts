import { Timestamp } from "server/misc/Timestamp";

/**
 * Message that is sent from PhPredictionServiceImpl to PhPredictionWorkerThread
 * and indicates that a prediction of min-ph is needed for the given closing state.
 */
export interface MinPhPredictionRequest {
    type: 'min-ph-prediction-request';
    co2ClosingState: Co2ClosingState;
}

/**
 * Possible messages that PhPredictionServiceImpl can send to PhPredictionWorkerThread.
 */
export type MessageToPhPredictionWorker = MinPhPredictionRequest;

/**
 * Message that is sent from PhPredictionWorkerThread to PhPredictionServiceImpl
 * and indicates that a prediction is done with the given value.
 */
export interface MinPhPredictionResponse {
    type: 'min-ph-prediction-response';
    minPhPrediction: number;
    requestTimestamp: Timestamp;
}

/**
 * Possible messages that PhPredictionWorkerThread can send to PhPredictionServiceImpl.
 */
export type MessageFromPhPredictionWorker = MinPhPredictionResponse;

/**
 * Origin of the data sample. We can initially train using data from other instance
 * and then gradually replace them by data from this instance..
 */
export enum Co2ClosingStateOrigin {
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
export interface Co2ClosingState {
    origin: Co2ClosingStateOrigin,

    // State at the moment of close operation - - - - - - - -

    // Time (unix, seconds).
    closeTime: number;

    // Ph values
    ph600AtClose: number;
    ph600OffsetsBeforeClose: number[];
    ph60OffsetsBeforeClose: number[];

    // Close-action output (result of close operation) - - - - - - - 
    minPh600OffsetAfterClose: number;
};

/**
 * Attempts to create Co2ClosingState for the time t.
 */
export function createCo2ClosingState(params: {
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

