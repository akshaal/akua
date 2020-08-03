import "reflect-metadata";

import expect from "expect";
import TimeService from "server/service/TimeService";
import { AveragingWindow } from "./AveragingWindow";

class TimeServiceMockImpl implements TimeService {
    now: number = 0;

    nowTimestamp(): readonly [number, number] {
        const secs = Math.floor(this.now);
        return [secs, (this.now - secs) * 1e9];
    }

    nowRoundedSeconds(): number {
        return Math.round(this.now);
    }
}

describe('AveragingWindow', () => {
    for (var offsetX = 0; offsetX < 10; offsetX++) {
        const offset = offsetX; // capture mutable variable

        it(`should work as expected on low sample frequency (offset=${offset})`, () => {
            const timeService = new TimeServiceMockImpl();
            const avg = new AveragingWindow({
                windowSpanSeconds: 1,
                sampleFrequency: 2,
                timeService
            });

            timeService.now = -100;
            for (var o = 0; o < offset; o++) {
                avg.add(1000);
            }

            timeService.now = 0;

            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);

            avg.add(0);
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(0);

            avg.add(10);
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(5);

            timeService.now = 0.5;
            avg.add(20);
            expect(avg.getCount()).toStrictEqual(3);
            expect(avg.get()).toStrictEqual(10);

            timeService.now = 0.9;
            avg.add(14);
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // we cache values for 0.3 seconds, so this one must not trigger expiration of old stuff
            timeService.now = 1.1;
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // Some values are expired
            timeService.now = 1.3;
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(17);

            // One more value expired
            timeService.now = 1.8;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(14);

            // Last value is supposed to be expired but we cache old result
            timeService.now = 2;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(14);

            // Last value is expired
            timeService.now = 2.2;
            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);
        });

        it(`should work as expected on higher sample frequency (offset=${offset})`, () => {
            const timeService = new TimeServiceMockImpl();
            const avg = new AveragingWindow({
                windowSpanSeconds: 1,
                sampleFrequency: 4,
                timeService
            });

            timeService.now = -100;
            for (var o = 0; o < offset; o++) {
                avg.add(1000);
            }

            timeService.now = 0;

            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);

            avg.add(0);
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            avg.add(10);
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(5);

            timeService.now = 0.5;
            avg.add(20);
            expect(avg.getCount()).toStrictEqual(3);
            expect(avg.get()).toStrictEqual(10);

            timeService.now = 0.9;
            avg.add(14);
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // we cache values for 0.3 seconds, so this one must not trigger expiration of old stuff
            timeService.now = 1.1;
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // Some values are expired
            timeService.now = 1.3;
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(17);

            // One more value expired
            timeService.now = 1.8;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is supposed to be expired but we cache old result
            timeService.now = 2;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is expired (this test is quite important for high frequency)
            timeService.now = 2.2;
            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);
        });

        it(`should work as expected on even more higher sample frequency (offset=${offset})`, () => {
            const timeService = new TimeServiceMockImpl();
            const avg = new AveragingWindow({
                windowSpanSeconds: 1,
                sampleFrequency: 6,
                timeService
            });

            timeService.now = -100;
            for (var o = 0; o < offset; o++) {
                avg.add(1000);
            }

            timeService.now = 0;

            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);

            avg.add(0);
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            avg.add(10);
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            timeService.now = 0.5;
            avg.add(20);
            expect(avg.getCount()).toStrictEqual(3);
            expect(avg.get()).toStrictEqual(10);

            timeService.now = 0.9;
            avg.add(14);
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // we cache values for 0.3 seconds, so this one must not trigger expiration of old stuff
            timeService.now = 1.1;
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // Some values are expired
            timeService.now = 1.3;
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            // One more value expired
            timeService.now = 1.8;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is supposed to be expired but we cache old result
            timeService.now = 2;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is expired (this test is quite important for high frequency)
            timeService.now = 2.2;
            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);
        });

        it(`should work as expected on higher than even more higher sample frequency (offset=${offset})`, () => {
            const timeService = new TimeServiceMockImpl();
            const avg = new AveragingWindow({
                windowSpanSeconds: 1,
                sampleFrequency: 8,
                timeService
            });

            timeService.now = -100;
            for (var o = 0; o < offset; o++) {
                avg.add(1000);
            }

            timeService.now = 0;

            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);

            avg.add(0);
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            avg.add(10);
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            timeService.now = 0.5;
            avg.add(20);
            expect(avg.getCount()).toStrictEqual(3);
            expect(avg.get()).toStrictEqual(null);

            timeService.now = 0.9;
            avg.add(14);
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // we cache values for 0.3 seconds, so this one must not trigger expiration of old stuff
            timeService.now = 1.1;
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(11);

            // Some values are expired
            timeService.now = 1.3;
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            // One more value expired
            timeService.now = 1.8;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is supposed to be expired but we cache old result
            timeService.now = 2;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is expired (this test is quite important for high frequency)
            timeService.now = 2.2;
            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);
        });

        it(`should work as expected on even higher than even more higher sample frequency (offset=${offset})`, () => {
            const timeService = new TimeServiceMockImpl();
            const avg = new AveragingWindow({
                windowSpanSeconds: 1,
                sampleFrequency: 10,
                timeService
            });

            timeService.now = -100;
            for (var o = 0; o < offset; o++) {
                avg.add(1000);
            }

            timeService.now = 0;

            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);

            avg.add(0);
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            avg.add(10);
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            timeService.now = 0.5;
            avg.add(20);
            expect(avg.getCount()).toStrictEqual(3);
            expect(avg.get()).toStrictEqual(null);

            timeService.now = 0.9;
            avg.add(14);
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(null);

            // we cache values for 0.3 seconds, so this one must not trigger expiration of old stuff
            timeService.now = 1.1;
            expect(avg.getCount()).toStrictEqual(4);
            expect(avg.get()).toStrictEqual(null);

            // Some values are expired
            timeService.now = 1.3;
            expect(avg.getCount()).toStrictEqual(2);
            expect(avg.get()).toStrictEqual(null);

            // One more value expired
            timeService.now = 1.8;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is supposed to be expired but we cache old result
            timeService.now = 2;
            expect(avg.getCount()).toStrictEqual(1);
            expect(avg.get()).toStrictEqual(null);

            // Last value is expired (this test is quite important for high frequency)
            timeService.now = 2.2;
            expect(avg.getCount()).toStrictEqual(0);
            expect(avg.get()).toStrictEqual(null);
        });
    }
});