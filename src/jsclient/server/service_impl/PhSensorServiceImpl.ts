import { injectable } from "inversify";
import PhSensorService, { Ph } from "server/service/PhSensorService";
import AvrService, { AvrPhState } from "server/service/AvrService";
import { AveragingWindow } from "../misc/AveragingWindow";
import { Observable, BehaviorSubject } from "rxjs";

const KH = 4;
const PH_SAMPLE_FREQUENCY = 7; // How many measurements per second our AVR performs

interface Solution {
    a: number;
    b: number;
}

interface Calibration {
    v1: number;
    ph1: number;
    v2: number;
    ph2: number;
}

// TODO: Move to config
// 2020.03.26: 4.01=3.08v,             6.86=2.575
// 2020.05.17: 4.01=3.09559736896566v, 6.86=2.5856278083541535
const aqua1Calibration: Calibration = {
    ph1: 4.01,
    v1: 3.09559736896566,
    ph2: 6.86,
    v2: 2.5856278083541535
};

// Depends on configuration (environment).
const calibration = aqua1Calibration;

function findSolution(c: Calibration): Solution {
    // (%i2) solve ([a*v1 + b = ph1, a*v2 + b = ph2], [a, b]);
    //                          ph1 - ph2      ph2 v1 - ph1 v2
    // (%o2)               [[a = ---------, b = ---------------]]
    //                           v1 - v2           v1 - v2

    const a = (c.ph1 - c.ph2) / (c.v1 - c.v2);
    const b = (c.ph2 * c.v1 - c.ph1 * c.v2) / (c.v1 - c.v2);

    return { a, b };
}

const solution = findSolution(calibration);

function calcPhFromVoltage(voltage: number): number {
    return solution.a * voltage + solution.b;
}

// ==========================================================================================

class SensorProcessor {
    readonly values$ = new BehaviorSubject<Ph | null>(null);
    private _voltage60sWindow = new AveragingWindow(60, PH_SAMPLE_FREQUENCY);
    private _voltage600sWindow = new AveragingWindow(600, PH_SAMPLE_FREQUENCY);
    private _pendingAvrPhState?: AvrPhState;

    onNewAvrState(newState: AvrPhState) {
        const thisVoltageIsGood = newState.voltage < 4 && newState.voltage > 1 && !newState.badSamples;

        if (thisVoltageIsGood) {
            // This voltage is in the interval and doesn't contain averages values adjacent to bad samples

            if (this._pendingAvrPhState) {
                // We must add pending PH value because we know it's not adjacent to the invalid one

                this._voltage60sWindow.add(this._pendingAvrPhState.voltage);
                this._voltage600sWindow.add(this._pendingAvrPhState.voltage);
    
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
                    phBasedCo2: phValue600s ? (3.0 * KH * (10 ** (7.00 - phValue600s))) : null,
                    lastSensorState: this._pendingAvrPhState
                });
            }

            // Set new avr ph state as pending, because we must know that the next state is valid one
            this._pendingAvrPhState = newState;
        } else {
            // This case is invalid one. Thus we invalidate the previous one (instead of using it)
            // and completely ignore this one
            this._pendingAvrPhState = undefined;
        }
    }

    get(): Ph | null {
        return this.values$.value;
    }
}

@injectable()
export default class PhSensorServiceImpl extends PhSensorService {
    private _phProcessor = new SensorProcessor();

    constructor(_avrService: AvrService) {
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