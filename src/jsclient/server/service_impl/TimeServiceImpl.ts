import { injectable  } from "inversify";
import TimeService from "server/service/TimeService";
import { Timestamp } from "server/misc/Timestamp";
import { newTimestamp } from "server/misc/new-timestamp";

@injectable()
export default class TimeServiceImpl extends TimeService {
    nowTimestamp(): Timestamp {
        return newTimestamp();
    }

    nowRoundedSeconds(): number {
        return Math.round(new Date().getTime() / 1000.0);
    }
}
