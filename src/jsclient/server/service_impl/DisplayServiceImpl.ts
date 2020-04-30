import { injectable, postConstruct } from "inversify";
import DisplayService from "server/service/DisplayManagerService";
import logger from "server/logger";
import { config } from "server/config";
import { openNextionPort } from "server/nextion";
import type { Nextion } from "server/nextion/nextion";
import { recurrent } from "../misc/recurrent";
import { DisplayTextElement, DisplayPicElement, DisplayPic, TouchEvent } from "server/service/DisplayService";
import { Subject } from "rxjs";

// We do attempt to reopen the port every this number of milliseconds.
const AUTO_REOPEN_MILLIS = 1000;

// Attempt to resurrect writing if it stopped for a reason (should not happen, but ...)
const AUTO_WRITE = 1000;

// Refresh display value to help with display disconnects
const AUTO_REFRESH = 1000;

function pct2clrComp(pct: number, bits: number): number {
    if (pct < 0) {
        return 0;
    }

    const m = (1 << bits) - 1;

    if (pct > 100) {
        return m;
    }

    return Math.round(pct * m / 100);
}

// ==========================================================================================

const port = config.nextion.port;

@injectable()
export default class DisplayServiceImpl extends DisplayService {
    private _nextion?: Nextion;
    private _canWrite: boolean = false;

    // Let us maintain order of values we write (so we can cycle names we write). see _setValue
    private _valueNamesToWrite: string[] = [];

    // Let us maintain map from value name to the value itself.
    // This let us write newest version of value...
    private _values: { [key: string]: number | string } = {};

    readonly touchEvents$ = new Subject<TouchEvent>();

    @postConstruct()
    _init(): void {
        let opening = false;

        // Reopen port is needed
        recurrent(AUTO_REOPEN_MILLIS, () => {
            if (!opening && (!this._nextion || !this._nextion.isOpen)) {
                this._nextion = undefined;

                opening = true;
                openNextionPort(port).then(nextion => {
                    opening = false;
                    logger.info("Display: Connected to nextion display: " + port);
                    this._nextion = nextion;
                    this._setup_nextion();
                }).catch(reason => {
                    opening = false;
                    logger.error("Display: Failed to connect to nextion display!", { port, reason });
                });
            }
        });

        // Attempt writing (we don't really need this, but who knows)
        recurrent(AUTO_WRITE, () => {
            this._writeNextValue();
        });

        // We must periodically force-refresh display in case it was disconnected without noticing it
        recurrent(AUTO_REFRESH, () => {
            this._forceDisplayRefresh();
        });
    }

    setTextColor(element: DisplayTextElement, rgbPcts: Readonly<[number, number, number]>): void {
        const color16bit = (pct2clrComp(rgbPcts[0], 5) << 11) + (pct2clrComp(rgbPcts[1], 6) << 5) + pct2clrComp(rgbPcts[2], 5);
        this._setValueIfChanged(element + ".pco", color16bit);
    }

    setText(element: DisplayTextElement, value: string): void {
        this._setValueIfChanged(element + ".txt", value);
    }

    setPic(element: DisplayPicElement, pic: DisplayPic): void {
        this._setValueIfChanged(element + ".pic", pic);
    }

    private _setup_nextion(): void {
        if (!this._nextion) {
            return;
        }

        // Capture into local scope
        const nextion = this._nextion;

        // On error
        nextion.on("error", error => {
            logger.error("Display: Nextion error", { error });
            this._reconnect();
        });

        // On disconnect
        nextion.on("disconnected", () => {
            logger.error("Display: Nextion disconnected");
            this._reconnect();
        });

        // On touch event
        nextion.on("touchEvent", data => {
            this.touchEvents$.next({
                isRelease: data.releaseEvent
            });
        });

        // Allow write
        this._canWrite = true;

        // Setup
        this._setValueIfChanged("dim", 100);
        this._forceDisplayRefresh();
    }

    // Queue values for write (we need it sometimes because in case display we disconnected or something like this)
    private _forceDisplayRefresh() {
        for (const name in this._values) {
            if (this._valueNamesToWrite.indexOf(name) < 0) {
                this._valueNamesToWrite.push(name);
            }
        }
    }

    private _reconnect(): void {
        if (!this._nextion) {
            return;
        }

        if (this._nextion.isOpen) {
            this._nextion.close();
        }

        this._canWrite = false;
        this._nextion = undefined;
    }

    private _setValueIfChanged(name: string, value: number | string): void {
        if (this._values[name] === value) {
            return;
        }

        if (this._valueNamesToWrite.indexOf(name) < 0) {
            this._valueNamesToWrite.push(name);
        }

        this._values[name] = value;
        this._writeNextValue();
    }

    private _writeNextValue(): void {
        if (!this._nextion || !this._canWrite || this._valueNamesToWrite.length === 0) {
            return;
        }

        const name = this._valueNamesToWrite[0];
        let value = this._values[name];

        if (typeof value === "string") {
            value = "\"" + value + "\"";
        }

        this._canWrite = false;
        this._nextion.setValue(name, value).then(() => {
            logger.debug("Display: done writing", { name, value });

            // Remove from list of values to write
            // Note, that we don't remove from map of value, because we might use it later
            // for change detection.
            this._valueNamesToWrite.shift();

            this._canWrite = true;
            this._writeNextValue();
        }).catch(reason => {
            logger.error("Display: Failed set value!", { name, value, reason });
            this._reconnect();
        })
    }
}
