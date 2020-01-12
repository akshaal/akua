import { injectable } from "inversify";

import MetricsService from "./MetricsService";
import DisplayService from "./DisplayService";

@injectable()
export default class ServerServices {
    public constructor(
        public readonly metricsService: MetricsService,
        public readonly displayService: DisplayService
    ) {
    }
}