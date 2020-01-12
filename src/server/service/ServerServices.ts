import { injectable } from "inversify";

import MetricsService from "./MetricsService";

@injectable()
export default class ServerServices {
    public constructor(public readonly metricsService: MetricsService) {
    }
}