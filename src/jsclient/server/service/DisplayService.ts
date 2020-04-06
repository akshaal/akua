import { injectable } from "inversify";

export enum DisplayTextElement {
    AQUA_TEMP = "t0",
    PH = "t1",
    CASE_TEMP = "t2",
    CLOCK = "t3",
    CO2 = "t4"
};

export enum DisplayPicElement {
    ICON_0 = "p0",
    ICON_1 = "p1",
    ICON_2 = "p2",
    ICON_3 = "p3"
};

export enum DisplayPic {
    BLANK = 1,
    CO2_ON = 2,
    CO2_COOLDOWN = 3,
    DAY = 4,
    NIGHT = 5,
    ERROR = 6,
    FORCE = 7
};

@injectable()
export default abstract class DisplayService {
    abstract setText(element: DisplayTextElement, value: string): void;
    abstract setPic(element: DisplayPicElement, pic: DisplayPic): void;
}