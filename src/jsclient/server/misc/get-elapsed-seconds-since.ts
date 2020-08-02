import type { Timestamp } from "./Timestamp";

/**
 * Returns "now - since" in seconds.
 */
export function getElapsedSecondsSince({now, since}: {now: Timestamp, since: Timestamp}): number {
    const secs = now[0] - since[0];
    const nanoseconds = now[1] - since[1];
    return secs + nanoseconds / 1e9;
}