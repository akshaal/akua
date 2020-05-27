import { Timestamp } from "server/misc/Timestamp";
import { isPresent } from "server/misc/isPresent";

// More minutes, slower training
export const PH_PREDICTION_WINDOW_MINUTES = 15;

// 4 samples per minute (to match be able to used data fetched from prometheus)
export const PH_PREDICTION_WINDOW_LENGTH = 4 * PH_PREDICTION_WINDOW_MINUTES;

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

    /**
     * Time (unix, seconds).
     */
    closeTime: number;

    /**
     * How many seconds value has been open.
     */
    openedSecondsAgo: number;

    // values
    ph600AtClose: number;
    ph60Offset15sInterval: number[];
    temperature15sInterval: number[];
    dayLightOn15sInterval: boolean[];
    co2ValveOpen15sInterval: boolean[];

    // Close-action output (result of close operation) - - - - - - - 

    /**
     * Minimum PH reached after the CO2-valve was closed.
     */
    minPh600OffsetAfterClose: number;
};

/**
 * Attempts to create Co2ClosingState for the time t.
 */
export function createCo2ClosingState(params: {
    closeTime: number,
    openedSecondsAgo: number,
    minPh600: number,
    origin: Co2ClosingStateOrigin,
    getPh600(t: number): number | null | undefined,
    getPh60(t: number): number | null | undefined,
    getTemperature(t: number): number | null | undefined,
    isDayLightOn(t: number): boolean | null | undefined,
    isCo2ValveOpen(t: number): boolean | null | undefined,
}): Co2ClosingState | null {
    const {
        closeTime,
        openedSecondsAgo,
        getPh600,
        getPh60,
        origin,
        minPh600,
        getTemperature,
        isDayLightOn,
        isCo2ValveOpen
    } = params;

    const ph600AtClose = getPh600(closeTime);
    if (!isPresent(ph600AtClose)) {
        return null;
    }

    const ph60Offset15sInterval: number[] = [];
    const temperature15sInterval: number[] = [];
    const dayLightOn15sInterval: boolean[] = [];
    const co2ValveOpen15sInterval: boolean[] = [];

    for (var s = 0; s < PH_PREDICTION_WINDOW_LENGTH; s += 1) {
        const m = closeTime - 15 * s;
        const ph60AtM = getPh60(m);
        const tempAtM = getTemperature(m);
        const dayLightOnAtM = isDayLightOn(m);
        const co2ValveOpenAtM = isCo2ValveOpen(m);

        if (!isPresent(ph60AtM) || ph60AtM < 5 || ph60AtM > 9) {
            return null;
        }

        if (!isPresent(tempAtM) || tempAtM < 10 || tempAtM > 40) {
            return null;
        }

        if (!isPresent(co2ValveOpenAtM) || !isPresent(dayLightOnAtM)) {
            return null;
        }

        ph60Offset15sInterval.unshift(ph60AtM - ph600AtClose);
        temperature15sInterval.unshift(tempAtM);
        dayLightOn15sInterval.unshift(dayLightOnAtM);
        co2ValveOpen15sInterval.unshift(co2ValveOpenAtM);
    }

    return {
        origin,
        openedSecondsAgo,
        closeTime,
        ph600AtClose,
        ph60Offset15sInterval,
        temperature15sInterval,
        dayLightOn15sInterval,
        co2ValveOpen15sInterval,
        minPh600OffsetAfterClose: minPh600 - ph600AtClose
    };
}

