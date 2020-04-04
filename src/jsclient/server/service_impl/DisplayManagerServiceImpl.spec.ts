import "reflect-metadata";
import * as assert from "assert";
import DisplayManagerServiceImpl from "./DisplayManagerServiceImpl";
import PhSensorService from "server/service/PhSensorService";
import TemperatureSensorService from "server/service/TemperatureSensorService";
import DisplayService, { DisplayElement } from "server/service/DisplayService";
import { mock, when, instance, anyString } from "ts-mockito";
import { TestScheduler } from "rxjs/testing";

class TestEnv {
    readonly testScheduler: TestScheduler = new TestScheduler((actual, expected) => assert.deepEqual(actual, expected));

    readonly displayServiceMock: DisplayService = mock<DisplayService>();
    readonly temperatureSensorServiceMock: TemperatureSensorService = mock<TemperatureSensorService>(4);
    readonly phSensorServiceMock: PhSensorService = mock<PhSensorService>();

    run(maxFrames: number): void {
        const service = new DisplayManagerServiceImpl(
            instance(this.temperatureSensorServiceMock),
            instance(this.displayServiceMock),
            instance(this.phSensorServiceMock),
            this.testScheduler
        );
        service._init();
        this.testScheduler.maxFrames = maxFrames;
        this.testScheduler.flush();
        service._destroy();
    }
}

describe('DisplayManagerServiceImpl', () => {
    it('should display observed values', () => {
        const env = new TestEnv();
        env.testScheduler.run(({ cold }) => {
            when(env.temperatureSensorServiceMock.aquariumTemperature$).thenReturn(
                cold("a|", {
                    a: null
                })
            );

            when(env.temperatureSensorServiceMock.caseTemperature$).thenReturn(
                cold("a|", {
                    a: null
                })
            );

            when(env.phSensorServiceMock.ph$).thenReturn(
                cold("a|", {
                    a: null
                })
            );

            when(env.displayServiceMock.setText(DisplayElement.CLOCK, anyString())).thenCall((_, str) => {
                console.log("Clock:", str);
            });

            when(env.displayServiceMock.setText(DisplayElement.AQUA_TEMP, anyString())).thenCall((_, str) => {
                console.log("Aqua temp:", str);
            });

            when(env.displayServiceMock.setText(DisplayElement.CASE_TEMP, anyString())).thenCall((_, str) => {
                console.log("Case temp:", str);
            });

            when(env.displayServiceMock.setText(DisplayElement.PH, anyString())).thenCall((_, str) => {
                console.log("PH:", str);
            });

            env.run(100);
        })
    });
});

