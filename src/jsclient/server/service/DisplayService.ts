import { injectable } from "inversify";

export enum DisplayTextElement {
    AQUA_TEMP = "t0",
    PH = "t1",
    CASE_TEMP = "t2",
    CLOCK = "t3",
    CO2 = "t4"
};

@injectable()
export default abstract class DisplayService {
    abstract setText(element: DisplayTextElement, value: string): void;
}