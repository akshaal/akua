import { injectable  } from "inversify";
import RandomNumberService from "server/service/RandomNumberService";

@injectable()
export default class RandomNumberServiceImpl extends RandomNumberService {
    next(): number {
        return Math.random();
    }
}
