import type { Timestamp } from "./Timestamp";

export function getElapsedSecondsSince(t: Timestamp): number {
    const delta = process.hrtime(t as any);
    const nanoseconds = delta[0] * 1e9 + delta[1];
    return nanoseconds / 1e9;
}