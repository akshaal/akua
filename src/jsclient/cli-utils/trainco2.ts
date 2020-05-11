import logger from "./logger";

logger.info("============================================================================");
logger.info("============================================================================");
logger.info(`Starting version ${config.version} in ${config.isProd ? 'production' : 'development'} mode.`);

if (config.isDev) {
    logger.debug("Obtained configuration:", config);
}
