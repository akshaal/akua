const MIN_PERCENT = 80;
const FORCE_FILTER_EVERY_SECS = 0.3;

type Timestamp = Readonly<[number, number]>;

interface Record {
    readonly t: Timestamp;
    readonly v: number;
}

function getElapsedSecondsSince(t: Timestamp): number {
    const delta = process.hrtime(t as any);
    const nanoseconds = delta[0] * 1e9 + delta[1];
    return nanoseconds / 1e9;
}

export class AveragingWindow {
    private _sum: number = 0;
    private _records: Record[] = [];
    private _dirty = false;
    private _last_filtering?: Timestamp;

    private readonly _minSamples = this.windowSpanSeconds * this.sampleFrequency * MIN_PERCENT / 100.0;

    constructor(private windowSpanSeconds: number, private sampleFrequency: number) {
    }

    getCount(): number {
        this.get(); // To fix dirty state
        return this._records.length;
    }

    get(): number | null {
        if (this._dirty || !this._last_filtering || getElapsedSecondsSince(this._last_filtering) > FORCE_FILTER_EVERY_SECS) {
            // We filter and recalculate sum again to avoid sum drifting away because of floating point number stuff
            this._sum = 0;
            this._records = this._records.filter(record => {
                const keep = getElapsedSecondsSince(record.t) < this.windowSpanSeconds;
                if (keep) {
                    this._sum += record.v;
                }
                return keep;
            });
            this._dirty = false;
            this._last_filtering = process.hrtime();
        }

        if (this._records.length >= this._minSamples) {
            return this._sum / this._records.length;
        }

        return null;
    }

    add(v: number): void {
        this._dirty = true;
        this._sum += v;
        this._records.push({
            t: process.hrtime(),
            v
        });
    }
}