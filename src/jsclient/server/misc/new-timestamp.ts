import type { Timestamp } from "../misc/Timestamp";

// TODO: Get rid of this one. We must use TimeService instead!
export function newTimestamp(): Timestamp {
    return process.hrtime();
}