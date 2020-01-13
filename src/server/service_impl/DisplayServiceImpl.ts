import { injectable } from "inversify";
import DisplayService from "server/service/DisplayService";
import logger from "server/logger";
import { config } from "server/config";
import { openNextionPort } from "server/nextion";
import type { Nextion } from "server/nextion/nextion";

// ==========================================================================================

const port = config.nextion.port;

@injectable()
export default class DisplayServiceImpl extends DisplayService {
    private _nextionP: Promise<Nextion> = openNextionPort(port);

    constructor() {
        super();

        this._nextionP.then(nextion => {
            logger.debug("Connected to nextion display", { port, nextion });
        }).catch(reason => {
            logger.error("Failed to connect to nextion display!", { port, reason });
        });
    }
}