import type { Timestamp } from "../misc/Timestamp";

export function newTimestamp(): Timestamp {
    return process.hrtime();
}