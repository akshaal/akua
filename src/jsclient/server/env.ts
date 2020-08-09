// Only for stuff that depends on environment variables!
// This class makes it easier to track what environment variables we use!

export const ENV_IOC_TOKEN = Symbol("env");

export interface Env {
    isDev: boolean;
    version?: string;
    nextionPort?: string;
    avrPort?: string;
    instanceName?: string; 
};

// Use this unless you CAN NOT use config service! Use config service whenever possible!
// (config service makes testing easier!)
export const realEnv: Env = {
    isDev: (process.env.NODE_ENV !== "production") || (process.env.AKUA_FORCE_DEV == "true"),
    version: process.env.AKUA_VERSION,
    nextionPort: process.env.AKUA_NEXTION_PORT,
    avrPort: process.env.AKUA_PORT,
    instanceName: process.env.AKUA_INSTANCE
};