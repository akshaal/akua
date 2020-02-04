import type { Timestamp } from "./Timestamp";

export function newTimestamp(): Timestamp {
    return process.hrtime();
}