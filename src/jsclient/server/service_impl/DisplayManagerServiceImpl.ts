import { injectable, postConstruct } from "inversify";
import DisplayManagerService from "server/service/DisplayManagerService";
import PhSensorService from "server/service/PhSensorService";
import DisplayService, { DisplayElement } from "server/service/DisplayService";

// Symbols compiled into "Aqua*" fonts: " .0123456789:⇡⇣"

@injectable()
export default class DisplayManagerServiceImpl extends DisplayManagerService {
    constructor(
        // TODO: private _temperatureSensorService: TemperatureSensorService,
        private _displayService: DisplayService,
        private _phSensorService: PhSensorService
    ) {
        super();
    }

    @postConstruct()
    _init() {
        // TODO: Map value after subscribe
        // TODO: Use timeoutWith (60 seconds) to map value to "" if nothing is received within timeout (60 seconds)
        this._phSensorService.ph$.subscribe(ph => {
            // TODO: !!!!!!!!!!!!!!!!!!!!!!
            const v = ph?.value60s;
            if (typeof v === "number") {
                this._displayService.setText(DisplayElement.PH, Math.round(v * 100) / 100 + "");
            } else {
                this._displayService.setText(DisplayElement.PH, "");
                // TODO: Also color
            }
        });
    }
}
