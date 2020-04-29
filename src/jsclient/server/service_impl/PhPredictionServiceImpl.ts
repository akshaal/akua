import { injectable, postConstruct } from "inversify";

@injectable()
export default class PhPredictionServiceImpl {
    @postConstruct()
    _init() {
    }
}
