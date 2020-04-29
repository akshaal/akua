import { injectable, postConstruct } from "inversify";

@injectable()
export default class Co2PredictionServiceImpl {
    @postConstruct()
    _init() {
    }
}