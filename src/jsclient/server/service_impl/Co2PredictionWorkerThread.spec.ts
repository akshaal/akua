import "reflect-metadata";
import { testModel } from "./Co2PredictionWorkerThread";

describe('Co2PredictionWorkerThread', async () => {
    it('do something', async () => {
        //service.prepareData();
        await testModel();
    }).timeout(10000);
});
