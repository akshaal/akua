import { injectable } from "inversify";
import { Timestamp } from "server/misc/Timestamp";

@injectable()
export default abstract class TimeService {
    /**
     * Returns highly precise timestamp. Useful for measuring intervals.
     */
    abstract nowTimestamp(): Timestamp;

    /**
     * Returns unix time rounded to seconds.
     */
    abstract nowRoundedSeconds(): number;
}
