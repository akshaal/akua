import { injectable } from "inversify";
import DatabaseService from "server/service/DatabaseService";
import { Co2ClosingState } from "server/service/PhPrediction";

import * as sqlite3 from "sqlite3";
import config from "server/config";
import logger from "server/logger";

function connectoToDatabase(fileName: string): sqlite3.Database {
    return new sqlite3.Database(
        config.database.baseDirectory + "/" + fileName,
        (err) => {
            if (err) {
                logger.error(`Unable to connect to ${fileName} database.`, { err });
            } else {
                logger.info(`Connected to the ${fileName} database.`);
            }
        }
    );
}

@injectable()
export default class DatabaseServiceImpl extends DatabaseService {
    private coClosingStateDb = connectoToDatabase(config.database.phClosingStateDbFileName);

    insertCo2ClosingState(newState: Co2ClosingState): void {

    }
}