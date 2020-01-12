// Based upon sourcecode and https://doc.esdoc.org/github.com/boneskull/nextion/
// Not all methods are typed here........

declare module 'nextion' {
    /**
     * Generic response or event from Nextion device.
     */
    export class Result {
        /**
         * Creates a {@link Result}.
         * @param code Decimal instruction code
         */
        constructor(public code: number);

        /**
         * Hexadecimal representation of instruction code.
         */
        get hex(): string;
    }

    /**
     * An "event" from a Nextion device.
     */
    export class EventResult extends Result {
        /**
         * Creates an EventResult.
         * @param code Decimal instruction code
         * @param data Any other data returned by result
         */
        constructor(public code: number, public data?);
    }

    /**
     * A response, either success or an error, from a command.
     */
    class ResponseResult extends Result {
        /**
         * Creates a ResponseResult.
         * @param code Decimal instruction code
         * @param data Any other data returned by result
         */
        constructor(public code: number, data?);
    }

    export default class Nextion extends EventEmitter {
        /**
         * Returns Nextion instance.
         * @param port 
         */
        static from(port: string): Promise<Nextion>;

        /**
         * Sets a local or global variable on the current page to a value
         * 
         * @param name Name of variable
         * @param value New variable value
         */
        setValue(name: string, value);

        /**
         * Get a value
         * 
         * @param name Name; can be `varName.val` or `component.txt`, etc.
         * @returns String or numeric data response (depending on variable's type)
         */
        getValue(name: string): Promise<ResponseResult<>, Error>;
    }
}
