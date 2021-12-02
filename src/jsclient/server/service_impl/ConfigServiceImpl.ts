import { injectable, inject } from "inversify";
import ConfigService, { Config, ValueDisplayConfig } from "server/service/ConfigService";
import { Env, ENV_IOC_TOKEN } from "server/env";

// ================================================================================================

@injectable()
export default class ConfigServiceImpl extends ConfigService {
    private _aquaX<T>(configs: { aqua1: T, aqua2: T }): T {
        if (this._env.instanceName === "aqua1") {
            return configs.aqua1;
        } else {
            return configs.aqua2;
        }
    }

    private readonly _aquaTemperatureDisplay: ValueDisplayConfig = {
        lowComfort: 23.6,
        highComfort: 24.5,
        lowTolerable: 22,
        highTolerable: 27,
        comfortRgbPcts: [100, 100, 100],
        lowRgbPcts: [0, 0, 100],
        highRgbPcts: [100, 0, 0],
    };

    private readonly _caseTemperatureDisplay: ValueDisplayConfig = {
        lowComfort: 17.5,
        highComfort: 40,
        lowTolerable: 10,
        highTolerable: 50,
        comfortRgbPcts: [100, 100, 100],
        lowRgbPcts: [0, 0, 100],
        highRgbPcts: [100, 0, 0],
    };

    private readonly _instanceName = this._env.instanceName || "unknown";

    // Top-level config value
    readonly config: Config = {
        isDev: this._env.isDev,
        version: this._env.version || "unknown",
        instanceName: this._instanceName,

        instanceId: this._aquaX({ aqua1: 1, aqua2: 2 }),

        bindOptions: {
            port: 3000,
            proto: "http",

            // Must bind to 0.0.0.0. Binding to 127.0.0.1 makes port-forwarding impossible in docker
            host: "0.0.0.0",
        },

        metrics: {
            eventLoopMonitoringResolutionMs: 50,
            autocleanupNotActiveForSeconds: 60 * 60 * 24 * 7,
            autocleanupIntervalSeconds: 60 * 60,
            sessionsCountingIntervalSeconds: 15,
        },

        nextion: {
            port: this._env.nextionPort || "/dev/ttyAMA0"
        },

        avr: {
            port: this._env.avrPort || "/dev/ttyUSB0"
        },

        aquaTemperatureDisplay: this._aquaTemperatureDisplay,

        caseTemperatureDisplay: this._aquaX({
            aqua1: this._caseTemperatureDisplay,
            aqua2: this._aquaTemperatureDisplay
        }),

        phSensorCalibration: this._aquaX({
            // USE akua_ph60s_voltage metric to find out voltage

            // 2020.03.26: 4.01=3.08v,                6.86=2.575
            // 2020.05.17: 4.01=3.09559736896566v,    6.86=2.5856278083541535
            // 2021.08.25: 4.01=3.109889539510251v    6.86=2.5899703130372185v
            aqua1: {
                ph1: 4.01,
                v1: 3.109889539510251,
                ph2: 6.86,
                v2: 2.5899703130372185
            },

            // 2020.07.28: 4.01=3.0807776310782327,   6.86=2.616606056118044
            // 2021.12.02: 4.01=3.0509069803017206,   6.86=2.5927038457935496
            aqua2: {
                ph1: 4.01,
                v1: 3.0509069803017206,
                ph2: 6.86,
                v2: 2.5927038457935496
            },
        }),

        phDisplay: {
            lowComfort: 6.8,
            highComfort: 7.2,
            lowTolerable: 6.5,
            highTolerable: 8,
            comfortRgbPcts: [100, 100, 100],
            lowRgbPcts: [100, 0, 0],
            highRgbPcts: [0, 0, 100],
        },

        co2Display: {
            lowComfort: 15,
            highComfort: 22,
            lowTolerable: 1,
            highTolerable: 30,
            comfortRgbPcts: [100, 100, 100],
            lowRgbPcts: [100, 100, 0],
            highRgbPcts: [100, 0, 0],
        },

        phController: {
            phTurnOnOffMargin: 0.1,
            minSafePh600: 6.8,
            minSafePh60: 6.6,
            dayStartPh: 6.8,
            dayEndPh: 7.2,

            // These parameters must be in harmony with the values in the firmware!
            normDayPrepareHour: 8,
            normDayStartHour: 10,
            normDayEndHour: 21,
            altDayPrepareHour: 8,
            altDayStartHour: 10,
            altDayEndHour: 18,
        },

        phClosingPrediction: {
            trainDatasetPercentage: 0.90,
        },

        database: {
            baseDirectory: this._env.isDev ? ("../../temp/db-" + this._instanceName) : "/var/akua/db",
            phClosingStateDbFileName: "ph-closing-state.db"
        },

        aquaEnv: {
            kh: 4,
            alternativeDay: this._aquaX({ aqua1: false, aqua2: false })
        }
    };

    constructor(@inject(ENV_IOC_TOKEN) private readonly _env: Env) {
        super();

        var errorsDetected = false;

        if (this.config.instanceName !== "aqua1" && this.config.instanceName !== "aqua2") {
            console.log("::ERROR:: UNKNOWN INSTANCE: " + this.config.instanceName);
        }

        if (this.config.avr.port === this.config.nextion.port) {
            console.log("::ERROR:: AVR and NEXTION ports configured to the same value: " + this.config.avr.port);
            errorsDetected = true;
        }

        if (errorsDetected) {
            console.log("F A T A L  E R R O R S  D E T E C T E D:  E X I S T I N G  N A O!");
            process.exit(2);
        }
    }
}
