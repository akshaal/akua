import type { Timestamp } from "./Timestamp";
import { getElapsedSecondsSince } from "./get-elapsed-seconds-since";
import TimeService from "server/service/TimeService";

const MIN_PERCENT = 50;
const FORCE_FILTER_EVERY_SECS = 0.3;

interface Record {
    readonly t: Timestamp;
    readonly v: number;
}

export class AveragingWindow {
    private _records: Record[] = [];
    private _last_filtering?: Timestamp;
    private _last_filtering_result: number | null = null;

    private readonly _minSamples = this._params.windowSpanSeconds * this._params.sampleFrequency * MIN_PERCENT / 100.0;

    constructor(private _params: { windowSpanSeconds: number, sampleFrequency: number, timeService: TimeService }) {
    }

    getCount(): number {
        this.get(); // To fix dirty state
        return this._records.length;
    }

    get(): number | null {
        const now = this._params.timeService.nowTimestamp();

        if (this._last_filtering_result === null || !this._last_filtering || getElapsedSecondsSince({ since: this._last_filtering, now }) > FORCE_FILTER_EVERY_SECS) {
            // We filter and recalculate sum again to avoid sum drifting away because of floating point number stuff
            var sum = 0;

            this._records = this._records.filter(record => {
                const keep = getElapsedSecondsSince({ since: record.t, now }) < this._params.windowSpanSeconds;
                if (keep) {
                    sum += record.v;
                }
                return keep;
            });
            this._last_filtering = this._params.timeService.nowTimestamp();
            this._last_filtering_result = sum / this._records.length;
        }

        if (this._records.length >= this._minSamples) {
            return this._last_filtering_result;
        }

        return null;
    }

    add(v: number): void {
        this._last_filtering_result = null;
        this._records.push({ t: this._params.timeService.nowTimestamp(), v });
    }
}
