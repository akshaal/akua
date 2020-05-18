import { injectable } from "inversify";

@injectable()
export default abstract class RandomNumberService {
    /**
     * Get next random number between 0 (inclusive) and 1 (exclusive).
     */
    abstract next(): number;
}
