import { injectable } from "inversify";
import PhSensorService, { Ph } from "server/service/PhSensorService";
import AvrService, { AvrPhState } from "server/service/AvrService";
import { AveragingWindow } from "./AveragingWindow";
import { Observable, BehaviorSubject } from "rxjs";

const Ph_SAMPLE_FREQUENCY = 7; // How many measurements per second our AVR performs

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

// 2020.03.26
const aqua1Calibration: Calibration = {
    ph1: 4.01,
    v1: 3.085,
    ph2: 6.86,
    v2: 2.575
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
    private _avg5sWindow = new AveragingWindow(5, Ph_SAMPLE_FREQUENCY);
    private _avg60sWindow = new AveragingWindow(60, Ph_SAMPLE_FREQUENCY);

    onNewAvrState(newState: AvrPhState) {
        if (newState.voltage < 5 && newState.voltage >= 0) {
            this._avg5sWindow.add(newState.voltage);
            this._avg60sWindow.add(newState.voltage);

            const avg5s = this._avg5sWindow.get();
            const avg60s = this._avg60sWindow.get();

            this.values$.next({
                voltage5s: avg5s ? Math.round(avg5s * 100) / 100.0 : avg5s,
                voltage5sSamples: this._avg5sWindow.getCount(),
                value60s: avg60s ? Math.round(calcPhFromVoltage(avg60s) * 1000) / 1000.0 : avg60s,
                value60sSamples: this._avg60sWindow.getCount(),
                lastSensorState: newState
            });
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