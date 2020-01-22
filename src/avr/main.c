#include <avr/interrupt.h>
#include <avr/io.h>

// Here is what we are going to use for communication using USB/serial port
// Frame format is 8N1 (8 bits, no parity, 1 stop bit)
#define AK_USART_BAUD_RATE  2400
#define AK_USART_FRAME_FORMAT ((1 << UCSZ00) | (1 << UCSZ01))

// 16Mhz, that's external oscillator on Nano V3.
// This doesn't configure it here, it just tells to our build system
// what we is actually using! Configuration is done using fuses (see flash-avr-fuses).
// Actual value might be a different one, one can actually measure it to provide
// some kind of accuracy (calibration) if needed.
X_CPU$(cpu_freq = 16000000);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PINS

// For safety reasons we set unused pins to read with pull up

// - left
X_UNUSED_PIN$(D3); // 1
X_UNUSED_PIN$(D4); // 2
// .................. 3 GND
// .................. 4 VCC
// .................. 5 GND
// .................. 6 VCC
// .................. 7 XT1
// .................. 8 XT2
X_UNUSED_PIN$(D5); // 9
X_UNUSED_PIN$(D6); // 10
X_UNUSED_PIN$(D7); // 11
X_UNUSED_PIN$(B0); // 12
X_UNUSED_PIN$(B1); // 13
X_UNUSED_PIN$(B2); // 14
X_UNUSED_PIN$(B3); // 15 MOSI
X_UNUSED_PIN$(B4); // 16 MISO
X_UNUSED_PIN$(D2); // 32
// USART ......... // 31 TxD
// USART ......... // 30 RxD
// .................. 29 Reset
X_UNUSED_PIN$(C5); // 28 ADC5
X_UNUSED_PIN$(C4); // 27 ADC4
X_UNUSED_PIN$(C3); // 26 ADC3
X_UNUSED_PIN$(C2); // 25 ADC2
X_UNUSED_PIN$(C1); // 24 ADC1
X_UNUSED_PIN$(C0); // 23 ADC0
X_UNUSED_PIN$(C7); // 22 ADC7
// .................. 21 AGND
// .................. 20 AREF
X_UNUSED_PIN$(C6); // 19 ADC6
// .................. 18 AVCC
// B5 - blue led .... 17 SCK


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Led pins

X_GPIO_OUTPUT$(blue_led, B5);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Watchdog

X_WATCHDOG$(8s);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Activity indication

// This piece of code will be executed in akat event/thread loop every 1/10 second.
// We use to turn the blue led ON and OFF
X_EVERY_DECISECOND$(counter) {
    STATIC_VAR$(u8 state, initial = 0);

    blue_led.set(state);
    state = !state;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// USART

X_INIT$(usart_init) {
    // Set baudrate
    const u16 ubrr = akat_cpu_freq_hz() / (AK_USART_BAUD_RATE * 8L) - 1;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr % 256;
    UCSR0A = 1 << U2X0;

    // Set frame format
    UCSR0C = AK_USART_FRAME_FORMAT;

    // Enable transmitter
    UCSR0B = 1 << TXEN0;
}

// ----------------------------------------------------------------
// This thread continuously writes current status into USART

THREAD$(usart_writer) {
    // ---- all variable in the thread must be static (green threads requirement)
    STATIC_VAR$(u8 xxxx); // TODO: REMOVE
    STATIC_VAR$(u8 byte_to_send);
    STATIC_VAR$(u8 byte_number_to_send);

    // ---- subroutines has can yield unlike functions

    // Wait until USART is ready to transmit next byte
    // from 'byte_to_send';
    SUB$(send_byte) {
        WAIT_UNTIL$(UCSR0A & (1 << UDRE0));
        UDR0 = byte_to_send;
    }

    // Send human readable presentation of byte_number_to_send
    SUB$(send_byte_number) {
        if (byte_number_to_send > 99) {
            u8 d = byte_number_to_send / 100;
            byte_to_send = '0' + d; CALL$(send_byte);
        }

        if (byte_number_to_send > 9) {
            u8 d = (byte_number_to_send % 100) / 10;
            byte_to_send = '0' + d; CALL$(send_byte);
        }

        u8 d = byte_number_to_send % 10;
        byte_to_send = '0' + d; CALL$(send_byte)
    }

    // Sends \r and \n
    SUB$(send_newline) {
        byte_to_send = '\r'; CALL$(send_byte);
        byte_to_send = '\n'; CALL$(send_byte);
    }

    // - - - - - - - - - - -
    // Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
    while(1) {
        // TODO: READ COMMANDS
        byte_to_send = 'A'; CALL$(send_byte);
        byte_number_to_send = xxxx; CALL$(send_byte_number);
        CALL$(send_newline);
        xxxx += 1;
    }
}

// ----------------------------------------------------------------
// This thread processes input from uart_input_buffer

// TODO: !!!


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Main

X_MAIN$() {
    // Enable interrupts
    sei();
}
