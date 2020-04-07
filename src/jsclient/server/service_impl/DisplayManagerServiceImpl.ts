import { injectable, postConstruct, optional, inject } from "inversify";
import DisplayManagerService from "server/service/DisplayManagerService";
import PhSensorService from "server/service/PhSensorService";
import DisplayService, { DisplayTextElement, DisplayPicElement, DisplayPic } from "server/service/DisplayService";
import { Observable, of, combineLatest, timer, SchedulerLike } from "rxjs";
import { map, timeoutWith, distinctUntilChanged, share, delay, startWith, repeat, skip, filter, switchMap, take, throttleTime } from 'rxjs/operators';
import { isPresent } from "./isPresent";
import TemperatureSensorService from "server/service/TemperatureSensorService";
import { Subscriptions } from "./Subscriptions";
import AvrService, { AvrState, AvrServiceState, LightForceMode } from "server/service/AvrService";
import config, { ValueDisplayConfig } from "server/config";

// Symbols compiled into "Aqua*" fonts: " .0123456789:⇡⇣"

const TIMEOUT_MS = 60_000;

interface FormatInfo<T> {
    readonly obs$: Observable<T>;
    readonly decimals: number;
    readonly displayElement: DisplayTextElement;
    readonly minDiff: number;
    readonly diffOffsetHours: number;
    readonly valueDisplayConfig: ValueDisplayConfig;

    getValue(t: T): number | null | undefined;
}

interface IconsState {
    error: boolean;
    forced: boolean;
    co2: 'on' | 'off' | 'waiting';
    light: 'day' | 'night' | 'unknown';
}

function nextIconSlot(slot: DisplayPicElement): DisplayPicElement | undefined {
    return (
        slot === DisplayPicElement.ICON_0 ? DisplayPicElement.ICON_1 :
            slot === DisplayPicElement.ICON_1 ? DisplayPicElement.ICON_2 :
                slot === DisplayPicElement.ICON_2 ? DisplayPicElement.ICON_3 : undefined
    );
}

function asIconsState(avrState?: AvrState, avrServiceState?: AvrServiceState): IconsState {
    const co2 =
        avrState?.co2ValveOpen ? 'on' :
            (avrState?.co2IsRequired && avrState?.co2day && avrState?.co2CooldownSeconds) ? 'waiting' : 'off';

    const light =
        avrState?.light?.dayLightOn ? 'day' :
            avrState?.light?.nightLightOn ? 'night' : 'unknown';

    const error = (avrServiceState?.protocolVersionMismatch !== 0) || (avrServiceState?.serialPortIsOpen !== 1);

    return {
        forced: !!(avrState?.light.dayLightForced || avrState?.light.nightLightForced),
        error,
        co2,
        light
    };
}

// - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - -- - - - - - - -

function getColor(value: number | undefined | null, valueDisplayConfig: ValueDisplayConfig): Readonly<[number, number, number]> {
    if (!isPresent(value)) {
        return valueDisplayConfig.comfortRgbPcts;
    }

    if (value >= valueDisplayConfig.lowComfort && value <= valueDisplayConfig.highComfort) {
        return valueDisplayConfig.comfortRgbPcts;
    }

    if (value <= valueDisplayConfig.lowTolerable) {
        return valueDisplayConfig.lowRgbPcts;
    }

    if (value >= valueDisplayConfig.highTolerable) {
        return valueDisplayConfig.highRgbPcts;
    }

    if (value <= valueDisplayConfig.lowComfort) {
        const len = valueDisplayConfig.lowComfort - valueDisplayConfig.lowTolerable;
        const distToLowTolerable = value - valueDisplayConfig.lowTolerable;
        const t = distToLowTolerable / len;
        return [
            valueDisplayConfig.lowRgbPcts[0] + (valueDisplayConfig.comfortRgbPcts[0] - valueDisplayConfig.lowRgbPcts[0]) * t,
            valueDisplayConfig.lowRgbPcts[1] + (valueDisplayConfig.comfortRgbPcts[1] - valueDisplayConfig.lowRgbPcts[1]) * t,
            valueDisplayConfig.lowRgbPcts[2] + (valueDisplayConfig.comfortRgbPcts[2] - valueDisplayConfig.lowRgbPcts[2]) * t
        ];
    }

    const len = valueDisplayConfig.highTolerable - valueDisplayConfig.highComfort;
    const distToHighComfort = value - valueDisplayConfig.highComfort;
    const t = distToHighComfort / len;
    return [
        valueDisplayConfig.comfortRgbPcts[0] + (valueDisplayConfig.highRgbPcts[0] - valueDisplayConfig.comfortRgbPcts[0]) * t,
        valueDisplayConfig.comfortRgbPcts[1] + (valueDisplayConfig.highRgbPcts[1] - valueDisplayConfig.comfortRgbPcts[1]) * t,
        valueDisplayConfig.comfortRgbPcts[2] + (valueDisplayConfig.highRgbPcts[2] - valueDisplayConfig.comfortRgbPcts[2]) * t
    ];
}

// - - - - - - - - - - - - - -  - - - - - - - - - - - - - - - - - - - -- - - - - - - -

@injectable()
export default class DisplayManagerServiceImpl extends DisplayManagerService {
    private _subs = new Subscriptions();

    constructor(
        private readonly _temperatureSensorService: TemperatureSensorService,
        private readonly _displayService: DisplayService,
        private readonly _phSensorService: PhSensorService,
        private readonly _avrService: AvrService,
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
            valueDisplayConfig: config.aquaTemperatureDisplay,
            getValue: t => t?.value
        });

        // Case temperature
        this._formatFromObservable({
            displayElement: DisplayTextElement.CASE_TEMP,
            obs$: this._temperatureSensorService.caseTemperature$,
            decimals: 1,
            minDiff: 0.1,
            diffOffsetHours: 1,
            valueDisplayConfig: config.caseTemperatureDisplay,
            getValue: t => t?.value
        });

        // PH
        this._formatFromObservable({
            displayElement: DisplayTextElement.PH,
            obs$: this._phSensorService.ph$,
            decimals: 2,
            minDiff: 0.03,
            diffOffsetHours: 0.25, // 15 minutes
            valueDisplayConfig: config.phDisplay,
            getValue: ph => ph?.value600s
        });

        // CO2
        this._formatFromObservable({
            displayElement: DisplayTextElement.CO2,
            obs$: this._phSensorService.ph$,
            decimals: 0,
            minDiff: 1,
            diffOffsetHours: 0.25, // 15 minutes
            valueDisplayConfig: config.co2Display,
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

        // Icons
        this._subs.add(
            this._avrService.avrState$.pipe(
                skip(1), // to avoid hot value
                timeoutWith(TIMEOUT_MS, of(undefined), this._scheduler),
                map(avrState => asIconsState(avrState, this._avrService.getServiceState())),
                repeat(),
                distinctUntilChanged(),
                share()
            ).subscribe(iconsState => {
                let iconSlot: DisplayPicElement | undefined = DisplayPicElement.ICON_0;
                if (iconsState?.light === 'day') {
                    this._displayService.setPic(iconSlot, DisplayPic.DAY);
                    iconSlot = nextIconSlot(iconSlot);
                } else if (iconsState?.light === 'night') {
                    this._displayService.setPic(iconSlot, DisplayPic.NIGHT);
                    iconSlot = nextIconSlot(iconSlot);
                }

                if (iconSlot && iconsState?.co2 === 'on') {
                    this._displayService.setPic(iconSlot, DisplayPic.CO2_ON);
                    iconSlot = nextIconSlot(iconSlot);
                } else if (iconSlot && iconsState?.co2 === 'waiting') {
                    this._displayService.setPic(iconSlot, DisplayPic.CO2_COOLDOWN);
                    iconSlot = nextIconSlot(iconSlot);
                }

                if (iconSlot && iconsState?.forced) {
                    this._displayService.setPic(iconSlot, DisplayPic.FORCE);
                    iconSlot = nextIconSlot(iconSlot);
                }

                if (iconSlot && iconsState?.error) {
                    this._displayService.setPic(iconSlot, DisplayPic.ERROR);
                    iconSlot = nextIconSlot(iconSlot);
                }

                while (isPresent(iconSlot)) {
                    this._displayService.setPic(iconSlot, DisplayPic.BLANK);
                    iconSlot = nextIconSlot(iconSlot);
                }
            })
        );

        // Touch events
        this._subs.add(
            this._displayService.touchEvents$.pipe(
                filter(event => event.isRelease),
                throttleTime(500),
                switchMap( () =>
                    this._avrService.avrState$.pipe(
                        skip(1), // to avoid hot values
                        take(1), // We don't need many results
                        timeoutWith(1000, of(null))
                    )
                )
            ).subscribe(avrState => {
                if (isPresent(avrState)) {
                    if (avrState.light.dayLightForced || avrState.light.nightLightForced) {
                        this._avrService.forceLight(LightForceMode.NotForced);
                    } else if (avrState.light.dayLightOn) {
                        this._avrService.forceLight(LightForceMode.Night);
                    } else if (avrState.light.nightLightOn) {
                        this._avrService.forceLight(LightForceMode.Day);
                    }
                }
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
                    this._displayService.setTextColor(info.displayElement, getColor(val, info.valueDisplayConfig));
                } else {
                    this._displayService.setText(info.displayElement, "");
                }
            })
        );
    }
}
