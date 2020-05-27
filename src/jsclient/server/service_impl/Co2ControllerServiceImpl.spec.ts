import "reflect-metadata";
import { calcMinPhEquationParams, calcMinPh } from "./Co2ControllerServiceImpl";
import expect from "expect";
import { PhControllerConfig } from "server/config";

const phControllerConfig4Test: PhControllerConfig = {
    phTurnOnOffMargin: 0.1,
    minSafePh600: 6.8,
    minSafePh60: 6.5,
    dayPrepareHour: 8,
    dayStartHour: 10,
    dayEndHour: 22,
    dayStartPh: 6.8,
    dayEndPh: 7.2,
};

describe('Co2ControllerServiceImpl.calcMinPhEquationParams', () => {
    it('must calculate equation parameters', () => {
        const params = calcMinPhEquationParams(phControllerConfig4Test);
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
        const params = calcMinPhEquationParams(phControllerConfig4Test);

        const ph0 = calcMinPh(phControllerConfig4Test, params, phControllerConfig4Test.dayPrepareHour - 1);
        const ph1 = calcMinPh(phControllerConfig4Test, params, phControllerConfig4Test.dayPrepareHour);
        const ph2 = calcMinPh(phControllerConfig4Test, params, phControllerConfig4Test.dayStartHour);
        const ph3 = calcMinPh(phControllerConfig4Test, params, phControllerConfig4Test.dayEndHour);
        const ph4 = calcMinPh(phControllerConfig4Test, params, phControllerConfig4Test.dayEndHour + 1);

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
