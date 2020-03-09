import { injectable } from "inversify";

import MetricsService from "./MetricsService";
import DisplayService from "./DisplayService";
import AvrService from "./AvrService";
import ControlService from "./ControlService";

@injectable()
export default class ServerServices {
    public constructor(
        public readonly metricsService: MetricsService,
        public readonly displayService: DisplayService,
        public readonly avrService: AvrService,
        public readonly controlService: ControlService
    ) {
    }
}