import { injectable } from "inversify";
import DatabaseService from "server/service/DatabaseService";
import { Co2ClosingState } from "server/service/PhPrediction";

import * as sqlite3 from "sqlite3";
import config from "server/config";
import logger from "server/logger";
import * as BSON from "bson";

const sqlCreatePhClosingStatesTable: string = `
    CREATE TABLE IF NOT EXISTS states (
        close_time INTEGER NOT NULL PRIMARY KEY,
        origin INTEGER NOT NULL ,
        bson BLOB NOT NULL
    )
`;

function connectToDatabase(fileName: string): Promise<sqlite3.Database> {
    return new Promise(function (resolve, reject) {
        const db = new sqlite3.Database(
            config.database.baseDirectory + "/" + fileName,
            (err) => {
                if (err) {
                    logger.error(`Unable to connect to ${fileName} database.`, { err })
                    reject(err);
                } else {
                    logger.info(`Connected to the ${fileName} database.`);
                    resolve(db);
                }
            }
        )
    });
}

function run(db: sqlite3.Database, sql: string, params: any): Promise<void> {
    return new Promise(function (resolve, reject) {
        db.run(sql, params,
            (err) => {
                if (err) {
                    reject(err);
                } else {
                    resolve();
                }
            }
        )
    });
}

async function initDb(fileName: string, ...sqls: string[]): Promise<sqlite3.Database> {
    const db = await connectToDatabase(fileName);

    for (var sql of sqls) {
        await run(db, sql, {});
    }

    return db;
}

@injectable()
export default class DatabaseServiceImpl extends DatabaseService {
    private readonly co2ClosingStateDbPromise =
        initDb(
            config.database.phClosingStateDbFileName,
            sqlCreatePhClosingStatesTable
        );

    async insertCo2ClosingState(newState: Co2ClosingState): Promise<void> {
        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;

        return run(
            co2ClosingStateDb,
            "INSERT INTO states (close_time, origin, bson) VALUES ($closeTime, $origin, $bson)",
            {
                "$closeTime": newState.closeTime,
                "$origin": newState.origin,
                "$bson": BSON.serialize(newState)
            }
        );
    }
}