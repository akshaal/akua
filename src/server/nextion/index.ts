import { Nextion } from './nextion';
import { UART } from './uart';

/**
 * Instantiates a Nextion instance and fulfills a Promise
 * when it's listening for data.
 * @param uart UART instance
 */
function instantiate(uart: UART): Promise<Nextion> {
    return new Promise(resolve => {
        const nextion = new Nextion(uart, {}, () => {
            resolve(nextion);
        });
    });
}

/**
 * Create a Nextion instance.
 * @param port Name of port (`COM1`, `/dev/tty.usbserial`, etc.), `Serialport` instance or `Duplex` stream.  Omit for autodetection.
 * @returns {Promise<Nextion>} - Nextion instance
 */
export const openNextionPort = (port: string): Promise<Nextion> => UART.from(port).then(instantiate);
