import { injectable } from "inversify";

import MetricsService from "./MetricsService";
import AvrService from "./AvrService";
import DisplayManagerService from "./DisplayManagerService";
import Co2ControllerService from "./Co2ControllerService";

@injectable()
export default class ServerServices {
    public constructor(
        public readonly metricsService: MetricsService,
        public readonly displayManagerService: DisplayManagerService,
        public readonly co2ControllerService: Co2ControllerService,
        public readonly avrService: AvrService
    ) {
    }
}