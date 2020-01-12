import { injectable } from "inversify";
import DisplayService from "server/service/DisplayService";
import Nextion from 'nextion';
import logger from "server/logger";
import { config } from "server/config";

// ==========================================================================================

const port = config.nextion.port;

@injectable()
export default class DisplayServiceImpl extends DisplayService {
    private _nextionP: Promise<Nextion> = Nextion.from(port);

    constructor() {
        super();

        this._nextionP.then(nextion => {
            logger.debug("Connected to nextion display", { port, nextion });
        }).catch(reason => {
            logger.error("Failed to connect to nextion display!", { port, reason });
        });
    }
}