import { ProtoHostPort } from "server/service/ConfigService";

export function asUrl(options: ProtoHostPort): string {
    if ((options.port == 443 && options.proto == "https") || (options.port == 80 && options.proto == "http")) {
        return `${options.proto}://${options.host}`;
    }

    return `${options.proto}://${options.host}:${options.port}`;
}