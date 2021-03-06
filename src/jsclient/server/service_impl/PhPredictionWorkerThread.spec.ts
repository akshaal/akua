import "reflect-metadata";
import { Worker } from "worker_threads";
import { MinPhPredictionRequest, createCo2ClosingState, Co2ClosingStateOrigin, MessageFromPhPredictionWorker } from "../service/PhPrediction";
import { akDropTimeseriesLayer } from "./PhPredictionWorkerThread";
import * as tf from 'server/service_impl/tf';
import expect from "expect";
import ConfigServiceImpl from "./ConfigServiceImpl";
import { realEnv } from "server/env";

// TODO: Need a way to provide parentPort
// TODO: cleanup!

const config = new ConfigServiceImpl(realEnv).config;

describe('PhPredictionWorkerThread', () => {
    it('can start as worker and predict min-ph', (done) => {
        const worker = new Worker("./dist/server/service_impl/PhPredictionWorkerThread.js", {
            workerData: config
        });

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

describe('AkDropTimeseriesLayer', () => {
    it('can properly drop timeseries', () => {
        const l = akDropTimeseriesLayer({ dropHead: 2, dropTail: 3 });

        const input = tf.tensor3d(
            [
                [[1, 2], [3, 4], [5, 6], [7, 8], [9, 10], [11, 12], [13, 14], [15, 16]],
                [[10, 20], [30, 40], [50, 60], [70, 80], [90, 100], [110, 120], [130, 140], [150, 160]],
                [[100, 200], [300, 400], [500, 600], [700, 800], [900, 1000], [1100, 1200], [1300, 1400], [1500, 1600]],
                [[1000, 2000], [3000, 4000], [5000, 6000], [7000, 8000], [9000, 10000], [11000, 12000], [13000, 14000], [15000, 16000]],
            ]
        );

        const output = l.call(input).arraySync();

        expect(output).toStrictEqual([
            [[5, 6], [7, 8], [9, 10]],
            [[50, 60], [70, 80], [90, 100]],
            [[500, 600], [700, 800], [900, 1000]],
            [[5000, 6000], [7000, 8000], [9000, 10000]]
        ]);
    });

    it('can properly calculate output shape', () => {
        const l = akDropTimeseriesLayer({ dropHead: 2, dropTail: 3 });
        const outputShape = l.computeOutputShape([100, 50, 8]);
        expect(outputShape).toStrictEqual([100, 45, 8]);
    });
});