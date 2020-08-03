import type { Timestamp } from "./Timestamp";
import { getElapsedSecondsSince } from "./get-elapsed-seconds-since";
import TimeService from "server/service/TimeService";
import { isPresent } from "./isPresent";

const MIN_PERCENT = 50;
const FORCE_FILTER_EVERY_SECS = 0.3;

interface Record {
    readonly t: Timestamp;
    readonly v: number;
}

export class AveragingWindow {
    private _buf: (Record | null)[] = Array(Math.ceil(this._params.windowSpanSeconds * this._params.sampleFrequency) + 1).fill(null);
    private _lastFiltering?: Timestamp;
    private _lastFilteringResult: number | null = null;
    private _count = 0;
    private _emptyIdx = 0;
    private _headIdx = 0;

    private readonly _minSamples = this._params.windowSpanSeconds * this._params.sampleFrequency * MIN_PERCENT / 100.0;

    constructor(private _params: { windowSpanSeconds: number, sampleFrequency: number, timeService: TimeService }) {
    }

    getCount(): number {
        this.get(); // To fix dirty state
        return this._count;
    }

    get(): number | null {
        if (this._count === 0) {
            return null;
        }

        const now = this._params.timeService.nowTimestamp();

        if (this._lastFilteringResult === null || !this._lastFiltering || getElapsedSecondsSince({ since: this._lastFiltering, now }) > FORCE_FILTER_EVERY_SECS) {
            while (this._count > 0 && this._isExpired(this._buf[this._headIdx], now)) {
                // Remove!
                this._buf[this._headIdx] = null;
                this._headIdx = this._incBufIdx(this._headIdx);
                this._count -= 1;

                // Last result no longer valid!
                this._lastFilteringResult = null;
            }

            // No need to do anything is everything is removed
            if (this._count === 0) {
                return null;
            }

            // Do it only if previous result is invalid (something is removed (see above) or added (see add method))
            if (this._lastFilteringResult === null) {
                // We recalculate sum again to avoid sum drifting away because of floating point number stuff
                var sum = 0;
                var idx = this._headIdx;

                do {
                    sum += this._buf[idx]?.v || 0;
                    idx = this._incBufIdx(idx);
                } while (idx !== this._emptyIdx);    

                this._lastFiltering = now;
                this._lastFilteringResult = sum / this._count;
            }
        }

        if (this._count >= this._minSamples) {
            return this._lastFilteringResult;
        }

        return null;
    }

    add(v: number): void {
        this._lastFilteringResult = null;

        if (this._count === this._buf.length) {
            // No space left, we must increase buffer
            const newBuf = Array<Record | null>(this._buf.length * 2).fill(null);

            // Copy data to new buffer
            var newEmptyIdx = 0;
            do {
                newBuf[newEmptyIdx] = this._buf[this._headIdx];
                this._headIdx = this._incBufIdx(this._headIdx);
                newEmptyIdx += 1;
            } while (this._headIdx !== this._emptyIdx);

            this._headIdx = 0;
            this._emptyIdx = newEmptyIdx;
            this._buf = newBuf;
        }

        this._buf[this._emptyIdx] = { t: this._params.timeService.nowTimestamp(), v };
        this._emptyIdx = this._incBufIdx(this._emptyIdx);
        this._count += 1;
    }

    private _incBufIdx(idx: number): number {
        const newIdx = idx + 1;
        if (newIdx == this._buf.length) {
            return 0;
        }
        return newIdx;
    }

    private _isExpired(record: Record | null, now: Timestamp): boolean {
        if (!isPresent(record)) {
            return true;
        }

        return getElapsedSecondsSince({ since: record.t, now }) >= this._params.windowSpanSeconds
    }
}
