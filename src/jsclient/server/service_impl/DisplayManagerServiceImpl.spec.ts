import "reflect-metadata";
import DisplayManagerServiceImpl from "./DisplayManagerServiceImpl";
import PhSensorService, { Ph } from "server/service/PhSensorService";
import TemperatureSensorService, { Temperature } from "server/service/TemperatureSensorService";
import DisplayService, { DisplayElement } from "server/service/DisplayService";
import { mock, when, instance, anyString } from "ts-mockito";
import { TestScheduler } from "rxjs/testing";
import expect from "expect";

interface TestCaseSpec {
    readonly aquariumTemperatureSpec: string;
    readonly aquariumTemperatureValues: { [letter: string]: Temperature };

    readonly caseTemperatureSpec: string;
    readonly caseTemperatureValues: { [letter: string]: Temperature };

    readonly phSpec: string;
    readonly phValues: { [letter: string]: Ph };
}

class TestCase {
    readonly testScheduler: TestScheduler = new TestScheduler((actual, expected) => assert.deepEqual(actual, expected));

    readonly displayServiceMock: DisplayService = mock<DisplayService>();
    readonly temperatureSensorServiceMock: TemperatureSensorService = mock<TemperatureSensorService>(4);
    readonly phSensorServiceMock: PhSensorService = mock<PhSensorService>();

    readonly clocks: string[] = [];
    readonly phs: string[] = [];
    readonly caseTemps: string[] = [];
    readonly aquaTemps: string[] = [];

    constructor(private _spec: TestCaseSpec) { }

    runFrames(maxFrames: number): void {
        this.testScheduler.run(({ cold }) => {
            // Setup mocks using test spec

            when(this.temperatureSensorServiceMock.aquariumTemperature$).thenReturn(
                cold(this._spec.aquariumTemperatureSpec, this._spec.aquariumTemperatureValues)
            );

            when(this.temperatureSensorServiceMock.caseTemperature$).thenReturn(
                cold(this._spec.caseTemperatureSpec, this._spec.caseTemperatureValues)
            );

            when(this.phSensorServiceMock.ph$).thenReturn(
                cold(this._spec.phSpec, this._spec.phValues)
            );

            when(this.displayServiceMock.setText(DisplayElement.CLOCK, anyString())).thenCall((_, str) => {
                this.clocks.push(str);
            });

            when(this.displayServiceMock.setText(DisplayElement.AQUA_TEMP, anyString())).thenCall((_, str) => {
                this.aquaTemps.push(str);
            });

            when(this.displayServiceMock.setText(DisplayElement.CASE_TEMP, anyString())).thenCall((_, str) => {
                this.caseTemps.push(str);
            });

            when(this.displayServiceMock.setText(DisplayElement.PH, anyString())).thenCall((_, str) => {
                this.phs.push(str);
            });

            // Create service using mocks
            const service = new DisplayManagerServiceImpl(
                instance(this.temperatureSensorServiceMock),
                instance(this.displayServiceMock),
                instance(this.phSensorServiceMock),
                this.testScheduler
            );

            // Perform unit of work
            service._init();
            this.testScheduler.maxFrames = maxFrames;
            this.testScheduler.flush();
            service._destroy();
        })
    }
}

describe('DisplayManagerServiceImpl', () => {
    it('should start with empty values and clock', () => {
        const testCase = new TestCase({
            aquariumTemperatureSpec: "",
            aquariumTemperatureValues: {},

            caseTemperatureSpec: "",
            caseTemperatureValues: {},

            phSpec: "",
            phValues: {},
        });

        testCase.runFrames(0);

        expect(testCase.aquaTemps).toStrictEqual([""]);
        expect(testCase.caseTemps).toStrictEqual([""]);
        expect(testCase.phs).toStrictEqual([""]);
        expect(testCase.clocks).toHaveLength(1);
        expect(testCase.clocks[0]).toMatch(/^\d\d:\d\d\.? +$/);
    });

    it('should display values', () => {
        const testCase = new TestCase({
            aquariumTemperatureSpec: "aab",
            aquariumTemperatureValues: {
                a: { value: 25.421, valueSamples: 100, lastSensorState: null },
                b: { value: 24.152, valueSamples: 100, lastSensorState: null }
            },

            caseTemperatureSpec: "aaab",
            caseTemperatureValues: {
                a: { value: 31.775, valueSamples: 110, lastSensorState: null },
                b: { value: 30.121, valueSamples: 110, lastSensorState: null }
            },

            phSpec: "aab",
            phValues: {
                a: {
                    phBasedCo2: 9.271,
                    voltage5s: 3.123,
                    voltage5sSamples: 1000,
                    value60s: 7.123,
                    value60sSamples: 1000,
                    value600s: 7.277,
                    value600sSamples: 10000,
                    lastSensorState: null
                },
                b: {
                    phBasedCo2: 9.121,
                    voltage5s: 3.143,
                    voltage5sSamples: 1000,
                    value60s: 7.513,
                    value60sSamples: 1000,
                    value600s: 7.917,
                    value600sSamples: 10000,
                    lastSensorState: null
                }
            },
        });

        testCase.runFrames(10);

        expect(testCase.aquaTemps).toStrictEqual(['', '25.4', '24.2']);
        expect(testCase.caseTemps).toStrictEqual(['', '31.8', '30.1']);
        expect(testCase.phs).toStrictEqual(['', '7.28', '7.92']);
    });
});

