import { injectable } from "inversify";
import { Observable } from "rxjs";

@injectable()
export default abstract class PhPredictionService {
    /**
     * Observable of predictions of PH if we assume that CO2-valve is turned off
     * at this moment but it really open.
     * This is used to decide whether we should turn off CO2 now or later.
     * Last predicted value is emitted when CO2-value is already turned off.
     */
    readonly abstract minClosingPhPrediction$: Observable<number>;

    /**
     * How much time it took to perform the prediction.
     */
    readonly abstract minClosingPhPredictionTimeUsed$: Observable<number>;
}
