import { injectable } from "inversify";
import PhSensorService, { Ph } from "server/service/PhSensorService";
import AvrService, { AvrPhState } from "server/service/AvrService";
import { AveragingWindow } from "../misc/AveragingWindow";
import { Observable, BehaviorSubject } from "rxjs";
import { calcCo2DivKhFromPh } from "server/misc/calcCo2DivKhFromPh";
import config, { PhSensorCalibrationConfig } from "server/config";
import TimeService from "server/service/TimeService";

// How many measurements per second our AVR performs
const PH_SAMPLE_FREQUENCY = 7;

// How many adjacent measurements to skip before and after the one marked as 'bad' (with noise in it).
const PH_BAD_VALUE_ADJ_SKIPS = 20;

interface Solution {
    a: number;
    b: number;
}

function findSolution(c: PhSensorCalibrationConfig): Solution {
    // (%i2) solve ([a*v1 + b = ph1, a*v2 + b = ph2], [a, b]);
    //                          ph1 - ph2      ph2 v1 - ph1 v2
    // (%o2)               [[a = ---------, b = ---------------]]
    //                           v1 - v2           v1 - v2

    const a = (c.ph1 - c.ph2) / (c.v1 - c.v2);
    const b = (c.ph2 * c.v1 - c.ph1 * c.v2) / (c.v1 - c.v2);

    return { a, b };
}

const solution = findSolution(config.phSensorCalibration);

function calcPhFromVoltage(voltage: number): number {
    return solution.a * voltage + solution.b;
}

// ==========================================================================================

class SensorProcessor {
    readonly values$ = new BehaviorSubject<Ph | null>(null);
    private _pendingAvrPhStates: AvrPhState[] = [];
    private _numberOfStatesToSkip: number = 0;

    private readonly _voltage60sWindow = new AveragingWindow({
        windowSpanSeconds: 60,
        sampleFrequency: PH_SAMPLE_FREQUENCY,
        timeService: this._timeService
    });

    private readonly _voltage600sWindow = new AveragingWindow({
        windowSpanSeconds: 600,
        sampleFrequency: PH_SAMPLE_FREQUENCY,
        timeService: this._timeService
    });

    constructor(private _timeService: TimeService) {}

    // TODO: We also must skip next sample if the current one is a bad one!

    onNewAvrState(newState: AvrPhState) {
        const thisVoltageIsGood = newState.voltage < 4 && newState.voltage > 1 && !newState.badSamples;

        if (thisVoltageIsGood) {
            // This voltage is in the interval and doesn't contain averages values adjacent to bad samples

            if (this._numberOfStatesToSkip) {
                // There was a bad value recently... we must skip some states to avoid bad influences...
                this._numberOfStatesToSkip -= 1;
            } else {
                if (this._pendingAvrPhStates.length >= PH_BAD_VALUE_ADJ_SKIPS) {
                    // We must add pending PH value because we know it's not adjacent to the invalid one
                    const pendingAvrPhState = this._pendingAvrPhStates.shift();

                    if (pendingAvrPhState) {
                        this._voltage60sWindow.add(pendingAvrPhState.voltage);
                        this._voltage600sWindow.add(pendingAvrPhState.voltage);

                        const voltage60s = this._voltage60sWindow.get();
                        const voltage600s = this._voltage600sWindow.get();

                        const phValue600s = voltage600s ? Math.round(calcPhFromVoltage(voltage600s) * 1000) / 1000.0 : voltage600s;

                        this.values$.next({
                            voltage60s: voltage60s,
                            voltage60sSamples: this._voltage60sWindow.getCount(),
                            value60s: voltage60s ? Math.round(calcPhFromVoltage(voltage60s) * 1000) / 1000.0 : voltage60s,
                            value60sSamples: this._voltage60sWindow.getCount(),
                            value600s: phValue600s,
                            value600sSamples: this._voltage600sWindow.getCount(),
                            phBasedCo2: phValue600s ? (calcCo2DivKhFromPh(phValue600s) * config.aquaEnv.kh) : null,
                            lastSensorState: pendingAvrPhState
                        });
                    }
                }

                // Set new avr ph state as pending, because we must know that the next state is valid one
                this._pendingAvrPhStates.push(newState);
            }
        } else {
            // This case is invalid one. Thus we invalidate the previous one (instead of using it)
            // and completely ignore this one
            this._pendingAvrPhStates = [];
            this._numberOfStatesToSkip = PH_BAD_VALUE_ADJ_SKIPS;
        }
    }

    get(): Ph | null {
        return this.values$.value;
    }
}

@injectable()
export default class PhSensorServiceImpl extends PhSensorService {
    private readonly _phProcessor = new SensorProcessor(this._timeService);

    constructor(_avrService: AvrService, private _timeService: TimeService) {
        super();
        _avrService.avrState$.subscribe(avrState => {
            this._phProcessor.onNewAvrState(avrState.ph);
        });
    }

    get ph(): Ph | null {
        return this._phProcessor.get();
    }

    get ph$(): Observable<Ph | null> {
        return this._phProcessor.values$;
    }
}