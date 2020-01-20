#include <avr/interrupt.h>
#include <avr/io.h>

// 16Mhz, that's external oscillator on Nano V3.
// This doesn't configure it here, it just tells to our build system
// what we is actually using! Configuration is done using fuses (see flash-avr-fuses).
X_CPU$(cpu_freq = 16000000);

// - - - - - - - - -  - - - - - - - Led pins

X_GPIO_OUTPUT$(xxx_pin, B5);

// - - - - - - - - -  - - - - - - - Unused pins

// For safety reasons we set them to read with pull up
// X_UNUSED_PIN$(B1);

// - - - - - - - - -  - - - - - - - Main

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
