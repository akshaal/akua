import { injectable } from "inversify";

import MetricsService from "./MetricsService";
import AvrService from "./AvrService";
import DisplayManagerService from "./DisplayManagerService";

@injectable()
export default class ServerServices {
    public constructor(
        public readonly metricsService: MetricsService,
        public readonly displayManagerService: DisplayManagerService,
        public readonly avrService: AvrService
    ) {
    }
}