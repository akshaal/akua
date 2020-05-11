"use strict";
var __importDefault = (this && this.__importDefault) || function (mod) {
    return (mod && mod.__esModule) ? mod : { "default": mod };
};
Object.defineProperty(exports, "__esModule", { value: true });
const logger_1 = __importDefault(require("./logger"));
logger_1.default.info("============================================================================");
logger_1.default.info("============================================================================");
logger_1.default.info(`Starting version ${config.version} in ${config.isProd ? 'production' : 'development'} mode.`);
if (config.isDev) {
    logger_1.default.debug("Obtained configuration:", config);
}
