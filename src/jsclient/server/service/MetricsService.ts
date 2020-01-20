import { injectable } from "inversify";

@injectable()
export default abstract class MetricsService {
    public abstract observeSimpleMeasurement(target: string, delta: [number, number]): void;

    public abstract getContentType(): string;

    public abstract getMetrics(): string;
}