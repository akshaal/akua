#include <avr/interrupt.h>
#include <avr/io.h>

// TODO: ??????????????????????????????????????????
X_CPU$(cpu_freq = 1061658);

// - - - - - - - - -  - - - - - - - Led pins

X_GPIO_OUTPUT$(xxx_pin, B5);

// - - - - - - - - -  - - - - - - - Unused pins

// For safety reasons we set them to read with pull up
// X_UNUSED_PIN$(B1);

// - - - - - - - - -  - - - - - - - Main

/*FUNCTION$(void delay_approx300ms()) {
    for (u16 i = 0; i < 10000; i++) {
        akat_delay_us(30);
    }
    }*/

// Main
X_MAIN$() {
    sei();
    xxx_pin.set(1);
    while(1) {
        //delay_approx300ms();
        //xxx_pin.set(0);
        //delay_approx300ms();
    }
}
