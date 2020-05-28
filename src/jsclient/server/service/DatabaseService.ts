import { injectable } from "inversify";
import { Co2ClosingState } from "./PhPrediction";

export enum Co2ClosingStateType {
    ANY = 0,
    VALIDATION = 1,
    TRAINING = 2,
};

@injectable()
export default abstract class DatabaseService {
    abstract async insertCo2ClosingState(newState: Co2ClosingState, attrs: { isValidation: boolean }): Promise<void>;

    abstract async markCo2ClosingStatesAsTraining(stateTimes: Readonly<number[]>): Promise<void>;

    abstract async findCo2ClosingStates(type: Co2ClosingStateType): Promise<Readonly<Co2ClosingState[]>>;

    abstract async findCo2ClosingTimes(type: Co2ClosingStateType): Promise<number[]>;

    abstract async countCo2ClosingStates(type: Co2ClosingStateType): Promise<number>;
}
