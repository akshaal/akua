import { injectable } from "inversify";
import { Co2ClosingState } from "./PhPrediction";

@injectable()
export default abstract class DatabaseService {
    abstract async insertCo2ClosingState(newState: Co2ClosingState): Promise<void>;
}
