import { injectable } from "inversify";
import { Co2ClosingState } from "./PhPrediction";

@injectable()
export default abstract class DatabaseService {
    abstract insertCo2ClosingState(newState: Co2ClosingState): void;
}
