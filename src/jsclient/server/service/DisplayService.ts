import { injectable } from "inversify";

export enum DisplayElement {
    AQUA_TEMP = "t0",
    PH = "t1",
    CASE_TEMP = "t2",
    CLOCK = "t3"
};

@injectable()
export default abstract class DisplayService {
    abstract setText(element: DisplayElement, value: string): void;
}