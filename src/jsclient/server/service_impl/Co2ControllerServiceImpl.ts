import { injectable, postConstruct } from "inversify";
import PhSensorService from "server/service/PhSensorService";
import Co2ControllerService from "server/service/Co2ControllerService";
import { combineLatest, timer } from "rxjs";
import { map, distinctUntilChanged } from "rxjs/operators";
import AvrService, { Co2ValveOpenState } from "server/service/AvrService";

// AVR is expecting that we notify it every minute
const SEND_REQUIREMENTS_TO_AVR_EVERY_MS = 60_000;

// Lower PH, more dissolved CO2
const MIN_PH_FOR_CO2 = 6.82;

@injectable()
export default class Co2ControllerServiceImpl extends Co2ControllerService {
    constructor(private readonly _phSensorService: PhSensorService,
                private readonly _avrService: AvrService) {
        super();
    }

    @postConstruct()
    _init() {
        combineLatest([
            this._phSensorService.ph$.pipe(
                map(ph => ph?.value600s),
                distinctUntilChanged()
            ),
            timer(0, SEND_REQUIREMENTS_TO_AVR_EVERY_MS)
        ]).subscribe(([ph]) => {
            const co2Required = !!(ph && ph > MIN_PH_FOR_CO2);
            this._avrService.setCo2RequiredValveOpenState(
                co2Required ? Co2ValveOpenState.Open : Co2ValveOpenState.Closed
            );
        });
    }
}
