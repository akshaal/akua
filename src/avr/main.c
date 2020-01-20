#include <avr/interrupt.h>
#include <avr/io.h>

// 16Mhz, that's external oscillator on Nano V3.
// This doesn't configure it here, it just tells to our build system
// what we is actually using! Configuration is done using fuses (see flash-avr-fuses).
X_CPU$(cpu_freq = 16000000);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PINS

// For safety reasons we set them to read with pull up

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
X_UNUSED_PIN$(D1); // 31 TxD
X_UNUSED_PIN$(D0); // 30 RxD
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

X_GPIO_OUTPUT$(xxx_pin, B5);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Main

FUNCTION$(void delay_approx300ms()) {
    for (u16 i = 0; i < 10000; i++) {
        akat_delay_us(30);
    }
}

// Main
X_MAIN$() {
    sei();
    while(1) {
        xxx_pin.set(1);
        delay_approx300ms();
        delay_approx300ms();
        delay_approx300ms();
        xxx_pin.set(0);
        delay_approx300ms();
        delay_approx300ms();
        delay_approx300ms();
    }
}
