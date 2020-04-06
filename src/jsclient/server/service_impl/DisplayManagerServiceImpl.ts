import { injectable, postConstruct, optional, inject } from "inversify";
import DisplayManagerService from "server/service/DisplayManagerService";
import PhSensorService from "server/service/PhSensorService";
import DisplayService, { DisplayTextElement } from "server/service/DisplayService";
import { Observable, of, combineLatest, timer, SchedulerLike } from "rxjs";
import { map, timeoutWith, distinctUntilChanged, share, delay, startWith, repeat, skip } from 'rxjs/operators';
import { isPresent } from "./isPresent";
import TemperatureSensorService from "server/service/TemperatureSensorService";
import { Subscriptions } from "./Subscriptions";

// Symbols compiled into "Aqua*" fonts: " .0123456789:⇡⇣"

const TIMEOUT_MS = 60_000;

interface FormatInfo<T> {
    readonly obs$: Observable<T>;
    readonly decimals: number;
    readonly displayElement: DisplayTextElement;
    readonly minDiff: number;
    readonly diffOffsetHours: number;

    getValue(t: T): number | null | undefined;
}

@injectable()
export default class DisplayManagerServiceImpl extends DisplayManagerService {
    private _subs = new Subscriptions();

    constructor(
        private _temperatureSensorService: TemperatureSensorService,
        private readonly _displayService: DisplayService,
        private readonly _phSensorService: PhSensorService,
        @optional() @inject("scheduler") private readonly _scheduler: SchedulerLike
    ) {
        super();
    }

    // Used in unit testing
    _destroy() {
        this._subs.unsubscribeAll();
    }

    // Called by inversify
    @postConstruct()
    _init() {
        // Aquarium temperature
        this._formatFromObservable({
            displayElement: DisplayTextElement.AQUA_TEMP,
            obs$: this._temperatureSensorService.aquariumTemperature$,
            decimals: 1,
            minDiff: 0.15,
            diffOffsetHours: 1,
            getValue: t => t?.value
        });

        // Case temperature
        this._formatFromObservable({
            displayElement: DisplayTextElement.CASE_TEMP,
            obs$: this._temperatureSensorService.caseTemperature$,
            decimals: 1,
            minDiff: 0.1,
            diffOffsetHours: 1,
            getValue: t => t?.value
        });

        // PH
        this._formatFromObservable({
            displayElement: DisplayTextElement.PH,
            obs$: this._phSensorService.ph$,
            decimals: 2,
            minDiff: 0.03,
            diffOffsetHours: 0.25, // 15 minutes
            getValue: ph => ph?.value600s
        });

        // CO2
        this._formatFromObservable({
            displayElement: DisplayTextElement.CO2,
            obs$: this._phSensorService.ph$,
            decimals: 0,
            minDiff: 1,
            diffOffsetHours: 0.25, // 15 minutes
            getValue: ph => ph?.phBasedCo2
        });

        // Clock
        this._subs.add(
            timer(0, 50, this._scheduler).pipe(
                map(() => {
                    const d = new Date();
                    const hh = ("0" + d.getHours()).slice(-2);
                    const mm = ("0" + d.getMinutes()).slice(-2);
                    const s = d.getSeconds();

                    if (s % 2 === 0) {
                        return hh + ":" + mm + "     ";
                    } else {
                        return hh + ":" + mm + ".    ";
                    }
                }),
                distinctUntilChanged()
            ).subscribe(v => {
                this._displayService.setText(DisplayTextElement.CLOCK, v);
            })
        );
    }

    // Subscribes to the observable given in 'info' and formats its value using instructions from
    // the 'info'.
    private _formatFromObservable<T>(info: FormatInfo<T>): void {
        const vals$: Observable<number | null | undefined> = info.obs$.pipe(
            skip(1),
            map(info.getValue),
            timeoutWith(TIMEOUT_MS, of(null), this._scheduler),
            repeat(),
            distinctUntilChanged(),
            share()
        );

        const delayedVals$: Observable<number | null | undefined> = vals$.pipe(
            delay(info.diffOffsetHours * 60 * 60 * 1000, this._scheduler)
        );

        this._subs.add(
            combineLatest([
                vals$.pipe(startWith(null)),
                delayedVals$.pipe(startWith(null))
            ]).subscribe(([val, delayedVal]) => {
                if (isPresent(val)) {
                    let str = val.toFixed(info.decimals);

                    if (isPresent(delayedVal)) {
                        const diff = val - delayedVal;
                        if (Math.abs(diff) >= info.minDiff) {
                            str += diff < 0 ? "⇣" : "⇡";
                        }
                    }

                    this._displayService.setText(info.displayElement, str);
                } else {
                    this._displayService.setText(info.displayElement, "");
                }
            })
        );
    }
}
