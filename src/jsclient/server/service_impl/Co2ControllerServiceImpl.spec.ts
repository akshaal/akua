import "reflect-metadata";
import { calcMinPhEquationParams, calcMinPh } from "./Co2ControllerServiceImpl";
import expect from "expect";
import { PhControllerConfig } from "server/service/ConfigService";

const phControllerConfig4Test: PhControllerConfig = {
    phTurnOnOffMargin: 0.1,
    minSafePh600: 6.8,
    minSafePh60: 6.5,
    normDayPrepareHour: 8,
    normDayStartHour: 10,
    normDayEndHour: 22,
    altDayPrepareHour: 8,
    altDayStartHour: 10,
    altDayEndHour: 18,
    dayStartPh: 6.8,
    dayEndPh: 7.2,
};

describe('Co2ControllerServiceImpl.calcMinPhEquationParams', () => {
    it('must calculate equation parameters', () => {
        const params = calcMinPhEquationParams({ phControllerConfig: phControllerConfig4Test, altDay: false });
        expect(params).toStrictEqual({
            a: 0.0000016239643829730753,
            b: 6.76422980406646,
            c: 0.000006697281216491395,
            d: 6.799006035337263
        });
    });
});

describe('Co2ControllerServiceImpl.calcMinPh', () => {
    it('must calculate min allowed ph', () => {
        const solution = calcMinPhEquationParams({ phControllerConfig: phControllerConfig4Test, altDay: false });

        const ph0 = calcMinPh({config: phControllerConfig4Test, solution, hour: phControllerConfig4Test.normDayPrepareHour - 1, altDay: false});
        const ph1 = calcMinPh({config: phControllerConfig4Test, solution, hour: phControllerConfig4Test.normDayPrepareHour, altDay: false});
        const ph2 = calcMinPh({config: phControllerConfig4Test, solution, hour: phControllerConfig4Test.normDayStartHour, altDay: false});
        const ph3 = calcMinPh({config: phControllerConfig4Test, solution, hour: phControllerConfig4Test.normDayEndHour, altDay: false});
        const ph4 = calcMinPh({config: phControllerConfig4Test, solution, hour: phControllerConfig4Test.normDayEndHour + 1, altDay: false});

        function r(x: number | undefined): number | undefined {
            if (typeof x === 'undefined') {
                return undefined;
            }

            return Math.round(x * 10000) / 10000;
        }

        expect(ph0).toStrictEqual(undefined);
        expect(r(ph1)).toStrictEqual(phControllerConfig4Test.dayEndPh);
        expect(r(ph2)).toStrictEqual(phControllerConfig4Test.dayStartPh);
        expect(r(ph3)).toStrictEqual(phControllerConfig4Test.dayEndPh);
        expect(ph4).toStrictEqual(undefined);
    });
});
