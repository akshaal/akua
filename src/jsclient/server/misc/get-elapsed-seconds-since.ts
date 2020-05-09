import type { Timestamp } from "./Timestamp";

// TODO: This function must be removed and replaced by function that calculates diff between to
// TODO: timestamps. In order to make stuff testable.
export function getElapsedSecondsSince(t: Timestamp): number {
    const delta = process.hrtime(t as any);
    const nanoseconds = delta[0] * 1e9 + delta[1];
    return nanoseconds / 1e9;
}