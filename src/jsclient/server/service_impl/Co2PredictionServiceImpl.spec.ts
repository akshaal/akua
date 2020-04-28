import "reflect-metadata";
import Co2PredictionServiceImpl from "./Co2PredictionServiceImpl";

describe('Co2PredictionServiceImpl', async () => {
    it('do something', async () => {
        const service = new Co2PredictionServiceImpl();
        service._init();
        //service.prepareData();
        await service.test();
    }).timeout(10000);
});
