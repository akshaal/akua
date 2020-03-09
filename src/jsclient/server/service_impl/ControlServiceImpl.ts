import { injectable, postConstruct } from "inversify";
import ControlService from "server/service/ControlService";
import AvrService from "server/service/AvrService";

const REFRESH_MILLIS = 100;

@injectable()
export default class ControlServiceImpl extends ControlService {
    constructor(private _avrService: AvrService) {
        super();
    }

    @postConstruct()
    _init(): void {
        // Schedule new state calculation
        const refresher = () => {
            setTimeout(refresher, REFRESH_MILLIS);
            this._updateControlState();
        };
        refresher();
    }

    private _updateControlState(): void {
        const date = new Date();
        const dayLightSwitchOn = (date.getTime() / 1000) % 120 > 60;
        const nightLightSwitchOn = !dayLightSwitchOn;

        this._avrService.requestControlState({
            dayLightSwitchOn, nightLightSwitchOn
        });
    }
}