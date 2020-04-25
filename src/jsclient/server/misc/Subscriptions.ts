import { Subscription } from "rxjs";

export class Subscriptions {
    private array: Subscription[] = [];

    public add(sub: Subscription): void {
        this.array.push(sub);
    }

    public unsubscribeAll(): void {
        for (let sub of this.array) {
            sub.unsubscribe();
        }

        this.array = [];
    }
}
