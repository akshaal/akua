import { injectable } from "inversify";

export interface PhControlRange {
    phToTurnOff: number | undefined;
    phToTurnOn: number | undefined;
};

@injectable()
export default abstract class Co2ControllerService {
    abstract getPhControlRange(): PhControlRange;
}