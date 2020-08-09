import { injectable } from "inversify";
import DatabaseService, { Co2ClosingStateType } from "server/service/DatabaseService";
import { Co2ClosingState } from "server/service/PhPrediction";

import * as sqlite3 from "sqlite3";
import logger from "server/logger";
import * as BSON from "bson";
import _ from "lodash";
import ConfigService from "server/service/ConfigService";

@injectable()
export default class DatabaseServiceImpl extends DatabaseService {
    private readonly co2ClosingStateDbPromise =
        this._initDb(
            this._configService.config.database.phClosingStateDbFileName,
            `
                CREATE TABLE IF NOT EXISTS states (
                    close_time INTEGER NOT NULL PRIMARY KEY,
                    origin INTEGER NOT NULL ,
                    bson BLOB NOT NULL
                )
            `,
            "ALTER TABLE states ADD is_validation INTEGER NOT NULL DEFAULT 1"
        );

    constructor(private readonly _configService: ConfigService) {
        super();
    }

    async insertCo2ClosingState(newState: Co2ClosingState, attrs: { isValidation: boolean }): Promise<void> {
        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;

        return run(
            co2ClosingStateDb,
            "INSERT INTO states (close_time, origin, bson, is_validation) VALUES ($closeTime, $origin, $bson, $isValidation)",
            {
                "$closeTime": newState.closeTime,
                "$origin": newState.origin,
                "$bson": BSON.serialize(newState),
                "$isValidation": attrs.isValidation ? 1 : 0
            }
        );
    }

    async markCo2ClosingStatesAsTraining(stateTimes: Readonly<number[]>): Promise<void> {
        if (!stateTimes.length) {
            return;
        }

        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;
        const stateTimesCSL = _.join(stateTimes, ",");

        return run(
            co2ClosingStateDb,
            `UPDATE states SET is_validation = 0 WHERE close_time IN (${stateTimesCSL})`,
            {}
        )
    }

    async findCo2ClosingStates(type: Co2ClosingStateType): Promise<Readonly<Co2ClosingState[]>> {
        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;
        const rows = await all(co2ClosingStateDb, "SELECT * FROM states " + asCo2ClosingStateWhereClause(type), {});
        return rows.map(r => BSON.deserialize(r.bson));
    }

    async findCo2ClosingTimes(type: Co2ClosingStateType): Promise<number[]> {
        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;
        const rows = await all(co2ClosingStateDb, "SELECT close_time FROM states " + asCo2ClosingStateWhereClause(type), {});
        return rows.map(r => r.close_time);
    }

    async countCo2ClosingStates(type: Co2ClosingStateType): Promise<number> {
        const co2ClosingStateDb = await this.co2ClosingStateDbPromise;
        const row = await get(co2ClosingStateDb, "SELECT count(*) AS cnt FROM states " + asCo2ClosingStateWhereClause(type), {});
        return row.cnt;
    }

    private _connectToDatabase(fileName: string): Promise<sqlite3.Database> {
        const configService = this._configService;

        return new Promise(function (resolve, reject) {
            const db = new sqlite3.Database(
                configService.config.database.baseDirectory + "/" + fileName,
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

    private async _initDb(fileName: string, ...sqls: string[]): Promise<sqlite3.Database> {
        const db = await this._connectToDatabase(fileName);

        const dbVersion = (await get(db, "PRAGMA user_version", {})).user_version;

        var v = 0;

        for (var sql of sqls) {
            v += 1;

            if (v > dbVersion) {
                logger.info(`Performing upgrade of database ${fileName} to version ${v}`);
                await run(db, sql, {});
                await run(db, `PRAGMA user_version = ${v}`, {});
            }
        }

        return db;
    }
}

// ///////////////////////////////////////////////////////////////////////////////////////////////

function asCo2ClosingStateWhereClause(type: Co2ClosingStateType): string {
    switch (type) {
        case Co2ClosingStateType.ANY: return "";
        case Co2ClosingStateType.TRAINING: return "WHERE is_validation = 0";
        case Co2ClosingStateType.VALIDATION: return "WHERE is_validation = 1";
    };
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

function all(db: sqlite3.Database, sql: string, params: any): Promise<Readonly<any[]>> {
    return new Promise(function (resolve, reject) {
        db.all(sql, params,
            (err, rows) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(rows);
                }
            }
        )
    });
}

function get(db: sqlite3.Database, sql: string, params: any): Promise<any> {
    return new Promise(function (resolve, reject) {
        db.get(sql, params,
            (err, row) => {
                if (err) {
                    reject(err);
                } else {
                    resolve(row);
                }
            }
        )
    });
}

