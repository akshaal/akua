import "reflect-metadata";
import logger from "server/logger";
import config from "server/config";
import { exit } from "process";
import { createNewContainer } from "server/service_impl/ServerServicesImpl";
import { Co2ClosingState, createCo2ClosingState, Co2ClosingStateOrigin } from "server/service/PhPrediction";
import DatabaseService from "server/service/DatabaseService";
import { readFileSync } from "fs";

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Performing training of ph2 prediction by co2-closing state`);

if (!config.isDev) {
    logger.error("This script must not be started in production mode!");
    exit(-2);
}

const container = createNewContainer('cli-utils');
const databaseService = container.get(DatabaseService);

// ========================================================================================

// TODO: Remove
export function prepareData(): Co2ClosingState[] {
    const result: Co2ClosingState[] = [];

    logger.info("Preparing data from json files!");

    const ph600Map: { [k: number]: number } = {};
    const ph60Map: { [k: number]: number } = {};
    const openMap: { [k: number]: number } = {};
    const keys: number[] = [];

    const ph600sJson = JSON.parse(readFileSync("server/static-ui/ph600s.json").toString("UTF-8"));
    const ph60sJson = JSON.parse(readFileSync("server/static-ui/ph60s.json").toString("UTF-8"));
    const openJson = JSON.parse(readFileSync("server/static-ui/open.json").toString("UTF-8"));

    ph600sJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        ph600Map[k] = parseFloat(v[1]);
        keys.push(k);
    });

    console.log("Time points in ph600-map:", Object.keys(ph600Map).length);

    ph60sJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        ph60Map[k] = parseFloat(v[1]);
    });

    console.log("Time points in ph60-map:", Object.keys(ph60Map).length);

    openJson.data.result[0].values.map((v: [number, string]) => {
        const k = Math.round(v[0]);
        openMap[k] = parseFloat(v[1]);
    });

    console.log("Time points in open-map:", Object.keys(openMap).length);

    for (const k of [...keys]) {
        if (ph600Map[k] === undefined || ph60Map[k] === undefined || openMap[k] === undefined) {
            const kI = keys.indexOf(k);
            console.log(kI);
            keys.splice(kI, 1);
        }
    }

    console.log("Common time points:", keys.length);

    // ---------------------------------------------------------------------------
    // Find points of valve turn-off

    var prevValveOpen = openMap[keys[0]];
    var valveCloseK: number = 0;
    var valveOpenedSecondsBeforeClose: number = 0;
    var valveOpenK: number = 0;
    var minPh600: number = 0;

    for (const k of keys) {
        const valveOpen = openMap[k];

        if (valveOpen === 0) {
            // Valve is closed, find the lowest value for ph600
            if (minPh600 > ph600Map[k]) {
                minPh600 = ph600Map[k];
            }
        }

        if (valveOpen === prevValveOpen) {
            // No changes in valve state
            continue;
        }

        prevValveOpen = valveOpen;

        if (valveOpen === 0) {
            // Valve is just closed
            valveCloseK = k;
            minPh600 = ph600Map[k];
            valveOpenedSecondsBeforeClose = k - valveOpenK;
            valveOpenK = 0;
        } else {
            valveOpenK = k;

            // Valve is just opened
            if (valveCloseK) {
                if (k - valveCloseK < 600) {
                    console.log("Too short closed interval", k - valveCloseK);
                    continue;
                }

                if (k - valveCloseK > (6 * 60 * 60)) {
                    console.log("Too long closed interval", (k - valveCloseK) / 60 / 60, "hours @ ", new Date(k * 1000).getHours(), "o'clock");
                    continue;
                }

                if (valveOpenedSecondsBeforeClose < 0) {
                    console.log("Negative open interval", valveOpenedSecondsBeforeClose);
                    continue;
                }

                if (valveOpenedSecondsBeforeClose > (60 * 60 * 12)) {
                    console.log("Unreal open interval", valveOpenedSecondsBeforeClose);
                    continue;
                }

                const state = createCo2ClosingState({
                    tClose: valveCloseK,
                    openedSecondsAgo: valveOpenedSecondsBeforeClose,
                    minPh600: minPh600,
                    origin: Co2ClosingStateOrigin.ThisInstance,
                    getPh600: (t: number) => ph600Map[t],
                    getPh60: (t: number) => ph60Map[t],
                });

                if (state) {
                    result.push(state);
                } else {
                    console.log("Bad data in interval");
                }
            }

            valveCloseK = 0;
        }
    }

    logger.info("count=" + result.length);

    return result;
}

export async function prepareDataAsync(): Promise<void> {
    const result = prepareData();

    for (var item of result) {
        await databaseService.insertCo2ClosingState(item);
    }

    logger.info("Done inserting new records! count=" + result.length);

    exit(0);
}


// ========================================================================================

// prepareDataAsync();
