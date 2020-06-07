import { AquaEnvConfig } from "server/config";

export function calcCo2FromPh(config: AquaEnvConfig, ph: number): number {
    return 3.0 * config.kh * (10 ** (7.00 - ph));
}
