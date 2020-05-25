import "reflect-metadata";
import { Worker } from "worker_threads";
import { MinPhPredictionRequest, createCo2ClosingState, Co2ClosingStateOrigin, MessageFromPhPredictionWorker } from "../service/PhPrediction";

// TODO: Need a way to provide parentPort

describe('PhPredictionWorkerThread', () => {
    it('can start as worker and predict min-ph', (done) => {
        const worker = new Worker("./dist/server/service_impl/PhPredictionWorkerThread.js");
        const now = 22341;
        const request: MinPhPredictionRequest = {
            type: 'min-ph-prediction-request',
            co2ClosingState: createCo2ClosingState({
                closeTime: now,
                openedSecondsAgo: 15,
                minPh600: 7, // doesn't matter
                origin: Co2ClosingStateOrigin.ThisInstance,
                getPh600: (t: number) => 7.2 - (now - t) / 600,
                getPh60: (t: number) => 7.18 - (now - t) / 600,
                getTemperature: (_t: number) => 24.2,
                isDayLightOn: (_t: number) => true,
                isCo2ValveOpen: (t: number) => t != now,
            }) as any
        };

        worker.on('message', (message: MessageFromPhPredictionWorker) => {
            worker.terminate();
            if (message.type === 'min-ph-prediction-response') {
                console.log(message.minPhPrediction);
                done();
            } else {
                done(new Error("Unexpected message: " + message.type));
            }
        });

        worker.postMessage(request);
    });
});
