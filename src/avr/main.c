
#include <avr/io.h>

// Day interval. Affects day-light and night light modes.
#define AK_DAY_START_HOUR 10
#define AK_DAY_DURATION_HOURS 10
#define AK_DAY_END_HOUR (AK_DAY_START_HOUR + AK_DAY_DURATION_HOURS)

// CO2-Day interval. Interval when it's allowed to feed CO2 to aquarium
#define AK_CO2_DAY_START_HOUR (AK_DAY_START_HOUR)
#define AK_CO2_DAY_END_HOUR (AK_DAY_END_HOUR - 1)

// Minimum hours when CO2 can be turned again after it was turned off
#define AK_CO2_OFF_HOURS_BEFORE_UNLOCKED 1

// Here is what we are going to use for communication using USB/serial port
// Frame format is 8N1 (8 bits, no parity, 1 stop bit)
#define AK_USART0_BAUD_RATE     9600
#define AK_USART0_FRAME_FORMAT  (H(UCSZ00) | H(UCSZ01))

// Size of buffer for bytes we receive from USART0/USB.
// RX-Interrupt puts bytes into the given ring buffer if there is space in it.
// A thread takes byte from the buffer and process it.
// Must be power of 2!
#define AK_USART0_RX_BUF_SIZE  128

// Number of debug bytes (data is written with '>' prefix into USART0)
#define AK_DEBUG_BUF_SIZE     64

// Maximum number of light forces within one hour
#define AK_MAX_LIGHT_FORCES_WITHIN_ONE_HOUR 10

// Maximum number of clock corrections within one hour
#define AK_MAX_CLOCK_CORRECTIONS_WITHIN_ONE_HOUR 10

// Maximum allowed clock drift before it's corrected
#define AK_MAX_CLOCK_DRIFT_DECISECONDS 5

// Maximum allowed delaying of clock correction (see above).
// This will be used if it's detected that correction
// will cause light changes... so we delay it until it doesn't cause it
#define AK_MAX_HARDLIMIT_CLOCK_DRIFT_DECISECONDS 20

// Number of deciseconds in a day
#define AK_NUMBER_DECISECONDS_IN_DAY (24L * 60L * 60L * 10L)

// Default time controller starts with.
// This is used to avoid situation when reset happens
// and Raspberry PI or other controller are unable to provide
// time. In this situtation cycle will be shifted, but basic light operation
// will continue to work.
// Default time is choosen in such a way that it doesn't trigger
// noisy switches (220v daylight contactor switching).
// This also helps to indicate that AVR has started (it will start with night light).
// So all this means that the default time is 3 hours after midnight.
#define AK_NUMBER_OF_CLOCK_DECISECONDS_AT_STARTUP (3L * 60L * 60L * 10L)

// 16Mhz, that's external oscillator on Mega 2560.
// This doesn't configure it here, it just tells to our build system
// what we is actually using! Configuration is done using fuses (see flash-avr-fuses).
// Actual value might be a different one, one can actually measure it to provide
// some kind of accuracy (calibration) if needed.
X_CPU$(cpu_freq = 16000000);


static const char HEX[16] = "0123456789abcdef";

// Must be the same enum as the enum in typescript with the same name
typedef enum {NotForced = 0, Day = 1, Night = 2} LightForceMode;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PINS

// For safety reasons we set unused pins to read with pull up

X_UNUSED_PIN$(G5); // 1    PG5 ( OC0B ) Digital pin 4 (PWM)
// USART0 / USB ..... 2    PE0 ( RXD0/PCINT8 ) Digital pin 0 (RX0)
// USART0 / USB ..... 3    PE1 ( TXD0 ) Digital pin 1 (TX0)
X_UNUSED_PIN$(E2); // 4    PE2 ( XCK0/AIN0 )
X_UNUSED_PIN$(E3); // 5    PE3 ( OC3A/AIN1 ) Digital pin 5 (PWM)
X_UNUSED_PIN$(E4); // 6    PE4 ( OC3B/INT4 ) Digital pin 2 (PWM)
X_UNUSED_PIN$(E5); // 7    PE5 ( OC3C/INT5 ) Digital pin 3 (PWM)
X_UNUSED_PIN$(E6); // 8    PE6 ( T3/INT6 )
X_UNUSED_PIN$(E7); // 9    PE7 ( CLKO/ICP3/INT7 )
// .................. 10   VCC
// .................. 11   GND
X_UNUSED_PIN$(H0); // 12   PH0 ( RXD2 ) Digital pin 17 (RX2)
X_UNUSED_PIN$(H1); // 13   PH1 ( TXD2 ) Digital pin 16 (TX2)
X_UNUSED_PIN$(H2); // 14   PH2 ( XCK2 )
X_UNUSED_PIN$(H3); // 15   PH3 ( OC4A ) Digital pin 6 (PWM)
X_UNUSED_PIN$(H4); // 16   PH4 ( OC4B ) Digital pin 7 (PWM)
X_UNUSED_PIN$(H5); // 17   PH5 ( OC4C ) Digital pin 8 (PWM)
X_UNUSED_PIN$(H6); // 18   PH6 ( OC2B ) Digital pin 9 (PWM)
X_UNUSED_PIN$(B0); // 19   PB0 ( SS/PCINT0 ) Digital pin 53 (SS)
X_UNUSED_PIN$(B1); // 20   PB1 ( SCK/PCINT1 ) Digital pin 52 (SCK)
X_UNUSED_PIN$(B2); // 21   PB2 ( MOSI/PCINT2 ) Digital pin 51 (MOSI)
X_UNUSED_PIN$(B3); // 22   PB3 ( MISO/PCINT3 ) Digital pin 50 (MISO)
X_UNUSED_PIN$(B4); // 23   PB4 ( OC2A/PCINT4 ) Digital pin 10 (PWM)
X_UNUSED_PIN$(B5); // 24   PB5 ( OC1A/PCINT5 ) Digital pin 11 (PWM)
X_UNUSED_PIN$(B6); // 25   PB6 ( OC1B/PCINT6 ) Digital pin 12 (PWM)
// BLUE LED ......... 26   PB7 ( OC0A/OC1C/PCINT7 ) Digital pin 13 (PWM)
X_UNUSED_PIN$(H7); // 27   PH7 ( T4 )
X_UNUSED_PIN$(G3); // 28   PG3 ( TOSC2 )
X_UNUSED_PIN$(G4); // 29   PG4 ( TOSC1 )
// .................. 30   RESET
// .................. 31   VCC
// .................. 32   GND
// .................. 33   XTAL2
// .................. 34   XTAL1
X_UNUSED_PIN$(L0); // 35   PL0 ( ICP4 ) Digital pin 49
X_UNUSED_PIN$(L1); // 36   PL1 ( ICP5 ) Digital pin 48
X_UNUSED_PIN$(L2); // 37   PL2 ( T5 ) Digital pin 47
X_UNUSED_PIN$(L3); // 38   PL3 ( OC5A ) Digital pin 46 (PWM)
X_UNUSED_PIN$(L4); // 39   PL4 ( OC5B ) Digital pin 45 (PWM)
X_UNUSED_PIN$(L5); // 40   PL5 ( OC5C ) Digital pin 44 (PWM)
X_UNUSED_PIN$(L6); // 41   PL6 Digital pin 43
X_UNUSED_PIN$(L7); // 42   PL7 Digital pin 42
X_UNUSED_PIN$(D0); // 43   PD0 ( SCL/INT0 ) Digital pin 21 (SCL)
X_UNUSED_PIN$(D1); // 44   PD1 ( SDA/INT1 ) Digital pin 20 (SDA)
// USART1 / CO2 ..... 45   PD2 ( RXDI/INT2 ) Digital pin 19 (RX1)
// USART1 / CO2 ..... 46   PD3 ( TXD1/INT3 ) Digital pin 18 (TX1)
X_UNUSED_PIN$(D4); // 47   PD4 ( ICP1 )
X_UNUSED_PIN$(D5); // 48   PD5 ( XCK1 )
X_UNUSED_PIN$(D6); // 49   PD6 ( T1 )
X_UNUSED_PIN$(D7); // 50   PD7 ( T0 ) Digital pin 38
X_UNUSED_PIN$(G0); // 51   PG0 ( WR ) Digital pin 41
X_UNUSED_PIN$(G1); // 52   PG1 ( RD ) Digital pin 40
X_UNUSED_PIN$(C0); // 53   PC0 ( A8 ) Digital pin 37
X_UNUSED_PIN$(C1); // 54   PC1 ( A9 ) Digital pin 36
X_UNUSED_PIN$(C2); // 55   PC2 ( A10 ) Digital pin 35
X_UNUSED_PIN$(C3); // 56   PC3 ( A11 ) Digital pin 34
X_UNUSED_PIN$(C4); // 57   PC4 ( A12 ) Digital pin 33
X_UNUSED_PIN$(C5); // 58   PC5 ( A13 ) Digital pin 32
X_UNUSED_PIN$(C6); // 59   PC6 ( A14 ) Digital pin 31
X_UNUSED_PIN$(C7); // 60   PC7 ( A15 ) Digital pin 30
// .................. 61   VCC
// .................. 62   GND
X_UNUSED_PIN$(J0); // 63   PJ0 ( RXD3/PCINT9 ) Digital pin 15 (RX3)
X_UNUSED_PIN$(J1); // 64   PJ1 ( TXD3/PCINT10 ) Digital pin 14 (TX3)
X_UNUSED_PIN$(J2); // 65   PJ2 ( XCK3/PCINT11 )
X_UNUSED_PIN$(J3); // 66   PJ3 ( PCINT12 )
X_UNUSED_PIN$(J4); // 67   PJ4 ( PCINT13 )
X_UNUSED_PIN$(J5); // 68   PJ5 ( PCINT14 )
X_UNUSED_PIN$(J6); // 69   PJ6 ( PCINT 15 )
X_UNUSED_PIN$(G2); // 70   PG2 ( ALE ) Digital pin 39
X_UNUSED_PIN$(A7); // 71   PA7 ( AD7 ) Digital pin 29
// CO2 switch         72   PA6 ( AD6 ) Digital pin 28
// Night light switch 73   PA5 ( AD5 ) Digital pin 27
// Main light switch  74   PA4 ( AD4 ) Digital pin 26
X_UNUSED_PIN$(A3); // 75   PA3 ( AD3 ) Digital pin 25
X_UNUSED_PIN$(A2); // 76   PA2 ( AD2 ) Digital pin 24
// DS18B20 Case ..... 77   PA1 ( AD1 ) Digital pin 23
// DS18B20 Aqua ..... 78   PA0 ( AD0 ) Digital pin 22
X_UNUSED_PIN$(J7); // 79   PJ7
// .................. 80   VCC
// .................. 81   GND
X_UNUSED_PIN$(K7); // 82   PK7 ( ADC15/PCINT23 ) Analog pin 15
X_UNUSED_PIN$(K6); // 83   PK6 ( ADC14/PCINT22 ) Analog pin 14
X_UNUSED_PIN$(K5); // 84   PK5 ( ADC13/PCINT21 ) Analog pin 13
X_UNUSED_PIN$(K4); // 85   PK4 ( ADC12/PCINT20 ) Analog pin 12
X_UNUSED_PIN$(K3); // 86   PK3 ( ADC11/PCINT19 ) Analog pin 11
X_UNUSED_PIN$(K2); // 87   PK2 ( ADC10/PCINT18 ) Analog pin 10
X_UNUSED_PIN$(K1); // 88   PK1 ( ADC9/PCINT17 ) Analog pin 9
X_UNUSED_PIN$(K0); // 89   PK0 ( ADC8/PCINT16 ) Analog pin 8
X_UNUSED_PIN$(F7); // 90   PF7 ( ADC7/TDI ) Analog pin 7
X_UNUSED_PIN$(F6); // 91   PF6 ( ADC6/TDO ) Analog pin 6
X_UNUSED_PIN$(F5); // 92   PF5 ( ADC5/TMS ) Analog pin 5
X_UNUSED_PIN$(F4); // 93   PF4 ( ADC4/TCK ) Analog pin 4
X_UNUSED_PIN$(F3); // 94   PF3 ( ADC3 ) Analog pin 3
X_UNUSED_PIN$(F2); // 95   PF2 ( ADC2 ) Analog pin 2
X_UNUSED_PIN$(F1); // 96   PF1 ( ADC1 ) Analog pin 1
// PH Meter ADC Port  97   PF0 ( ADC0 ) Analog pin 0 (marked as A0 on PCB, but F0 in code)
// .................. 98   AREF, Analog Reference
// .................. 99   GND
// .................. 100  AVCC


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Timers

// 16-bit Timer1 is used for 'X_EVERY_DECISECOND$'

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Support for simple debugging

GLOBAL$() {
    // Make volatile so we can use it from ISR (in theory)
    STATIC_VAR$(volatile u8 debug_bytes_buf[AK_DEBUG_BUF_SIZE], initial = {});
    STATIC_VAR$(volatile u8 debug_next_empty_idx);
    STATIC_VAR$(volatile u8 debug_next_read_idx);
    STATIC_VAR$(volatile u8 debug_overflow_count);
}

FUNCTION$(void add_debug_byte(const u8 b), unused) {
    u8 new_next_empty_idx = (debug_next_empty_idx + AKAT_ONE) & (AK_DEBUG_BUF_SIZE - 1);
    if (new_next_empty_idx == debug_next_read_idx) {
        debug_overflow_count += AKAT_ONE;
        // Don't let it overflow!
        if (!debug_overflow_count) {
            debug_overflow_count -= AKAT_ONE;
        }
    } else {
        debug_bytes_buf[debug_next_empty_idx] = b;
        debug_next_empty_idx = new_next_empty_idx;
    }
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Performance counter

// Measure how many iteration we are able to perform between decisecond ticks
// We use internal counter which we increment in every iteration of main loop
// and then we copy the counter to main_loop_iterations_in_last_decisecond
// every decisecond and reset the internal counter.
// Lowe the value in main_loop_iterations_in_last_decisecond, busyier the loop!

GLOBAL$() {
    STATIC_VAR$(u32 __current_main_loop_iterations);
    STATIC_VAR$(u32 main_loop_iterations_in_last_decisecond);
}

RUNNABLE$(performance_runnable) {
    __current_main_loop_iterations += 1;
}

X_EVERY_DECISECOND$(performance_ticker) {
    main_loop_iterations_in_last_decisecond = __current_main_loop_iterations;
    __current_main_loop_iterations = 0;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Control

X_GPIO_SAFE_OUTPUT$(day_light_switch, A4, safe_state = 0, state_timeout_deciseconds = 100);
X_GPIO_SAFE_OUTPUT$(night_light_switch, A5, safe_state = 0, state_timeout_deciseconds = 100);
X_GPIO_SAFE_OUTPUT$(co2_switch, A6, safe_state = 0, state_timeout_deciseconds = 100);

GLOBAL$() {
    // Clock used to calculate state
    STATIC_VAR$(u24 clock_deciseconds_since_midnight,
                initial = AK_NUMBER_OF_CLOCK_DECISECONDS_AT_STARTUP);

    // Last calculated drift of our clock comapred to data we got from server
    // This one is signed integer!
    STATIC_VAR$(i24 last_drift_of_clock_deciseconds_since_midnight,
                initial = 0);

    // Protects against spam / malfunction / misuse
    STATIC_VAR$(u8 light_forces_since_protection_stat_reset);
    STATIC_VAR$(u8 clock_corrections_since_protection_stat_reset);

    // These variables indicate correction stuff received from raspberry-pi (i.e. via USB)
    STATIC_VAR$(u8 received_clock0); // LSByte
    STATIC_VAR$(u8 received_clock1);
    STATIC_VAR$(u8 received_clock2, initial = 255); // MSByte. 255 means nothing is received

    // CO2 protection - - - 
    STATIC_VAR$(u24 co2_deciseconds_until_can_turn_on,
                initial = 0); // TODO: Change to (AK_CO2_OFF_HOURS_BEFORE_UNLOCKED * 60L * 60L * 10L)

    // Whether it's day or not as calculated by AVR's internal algorithm and interval
    STATIC_VAR$(u8 co2_calculated_day, initial = 0);
}

// State that raspberry pi CO2-controlling algorithm wants to set.
// It's expected that rpi-controller confirms the state every minute.
// But we tolerate restart of the rpi-controller within 15 minutes.
X_FLAG_WITH_TIMEOUT$(required_co2_switch_state, timeout = 15, unit = minutes);

// Forced states
X_FLAG_WITH_TIMEOUT$(day_light_forced, timeout = 10, unit = minutes);
X_FLAG_WITH_TIMEOUT$(night_light_forced, timeout = 10, unit = minutes);
X_FLAG_WITH_TIMEOUT$(co2_force_off, timeout = 10, unit = minutes);

X_EVERY_HOUR$(reset_protection_stats) {
    light_forces_since_protection_stat_reset = 0;
    clock_corrections_since_protection_stat_reset = 0;
}

// Called from uart-command-receiver if force-light command is received from raspberry-pi
// Note that if we force something, then it will be automatically reset by 
// X_FLAG_WITH_TIMEOUT after some interval. See above.
FUNCTION$(void force_light(const LightForceMode mode)) {
    if (mode == NotForced) {
        day_light_forced.set(0);
        night_light_forced.set(0);
        return;
    }

    if (light_forces_since_protection_stat_reset >= AK_MAX_LIGHT_FORCES_WITHIN_ONE_HOUR) {
        // TODO: Add statistics about how many forces was ignored!
        return;
    }

    if (mode == Day) {
        day_light_forced.set(1);
        night_light_forced.set(0);
    } else {
        day_light_forced.set(0);
        night_light_forced.set(1);
    }

    light_forces_since_protection_stat_reset += 1;
}

// Called from uart-command-receiver if set-co2 switch command is received from raspberry-pi
FUNCTION$(void update_co2_switch_state(const u8 new_state)) {
    required_co2_switch_state.set(new_state);
}

// - - - - - - - - - - - -  - - - - - - - ---- - - -- - - - -  - - - -
// - - - - - - - - - - - -  - - - - - - - ---- - - -- - - - -  - - - -
// - - - - - - - - - - - -  - - - - - - - ---- - - -- - - - -  - - - -
// Main function that performs state switching

FUNCTION$(u8 is_day(const u24 deciseconds_since_midnight)) {
    if (day_light_forced.is_set()) {
        return AKAT_ONE;
    }

    if (night_light_forced.is_set()) {
        return 0;
    }

    return (deciseconds_since_midnight >= (AK_DAY_START_HOUR * 60L * 60L * 10L)) && (deciseconds_since_midnight < (AK_DAY_END_HOUR * 60L * 60L * 10L));
}

X_EVERY_DECISECOND$(controller_tick) {
    // - - - - - - - - - - - - - - - -  CLOCK CORRECTION - - - - - - 
    u24 new_clock_deciseconds_since_midnight = clock_deciseconds_since_midnight + 1;
    if (new_clock_deciseconds_since_midnight >= AK_NUMBER_DECISECONDS_IN_DAY) {
        new_clock_deciseconds_since_midnight = 0;
    }

    u8 new_calculated_day_light_state = is_day(new_clock_deciseconds_since_midnight);

    if (received_clock2 != 255) {
        u24 received_clock = (((u24)received_clock2) << 16) + (((u24)received_clock1) << 8) + received_clock0;

        last_drift_of_clock_deciseconds_since_midnight = (i24)received_clock - (i24)clock_deciseconds_since_midnight;

        // Perform correction if drift is large than a limit
        if (clock_corrections_since_protection_stat_reset < AK_MAX_CLOCK_CORRECTIONS_WITHIN_ONE_HOUR) {
            if ((last_drift_of_clock_deciseconds_since_midnight >= AK_MAX_CLOCK_DRIFT_DECISECONDS) || (last_drift_of_clock_deciseconds_since_midnight <= -AK_MAX_CLOCK_DRIFT_DECISECONDS)) {
                // Perform correction if correction doesn't affect light state..
                // or if we can't delay correction any longer.
                const u8 corrected_calculated_day_light_state = is_day(new_clock_deciseconds_since_midnight);
                if ((corrected_calculated_day_light_state == new_calculated_day_light_state)
                        || (last_drift_of_clock_deciseconds_since_midnight >= AK_MAX_HARDLIMIT_CLOCK_DRIFT_DECISECONDS)
                        || (last_drift_of_clock_deciseconds_since_midnight <= -AK_MAX_HARDLIMIT_CLOCK_DRIFT_DECISECONDS)) {
                    // Perform correction...
                    clock_corrections_since_protection_stat_reset += 1;
                    new_clock_deciseconds_since_midnight = received_clock;
                    new_calculated_day_light_state = corrected_calculated_day_light_state;
                }
            }
        }
    }

    // We either have handled clock correction or ignored it.
    // Anyway, invalidate current correction data
    received_clock2 = 255;

    // Finally set clock to new clock value, either corrected or normal one
    clock_deciseconds_since_midnight = new_clock_deciseconds_since_midnight;

    // - - - - - - - - - - - - - - - -  CO2 - - - - - - 

    // New CO2 state if by default OFF
    u8 new_co2_state = 0;

    // Calculate day so it can be used also used for debugging
    co2_calculated_day =
        (clock_deciseconds_since_midnight >= (AK_CO2_DAY_START_HOUR * 60L * 60L * 10L))
            && (clock_deciseconds_since_midnight < (AK_CO2_DAY_END_HOUR * 60L * 60L * 10L));

    if (co2_deciseconds_until_can_turn_on) {
        // We can't feed CO2, because we can't yet... (it was turned off recently)
        co2_deciseconds_until_can_turn_on -= 1;
    } else {
        if (required_co2_switch_state.is_set()) {
            // New state will be whether it's in co2 day or not
            new_co2_state = co2_calculated_day && !co2_force_off.is_set();
        }
    }

    // - - - - - - - - - - - - - - - -  STATE CHANGING - - - - - - 

    // Light
    if (new_calculated_day_light_state) {
        day_light_switch.set(AKAT_ONE);
        night_light_switch.set(0);
    } else {
        day_light_switch.set(0);
        night_light_switch.set(AKAT_ONE);
    }

    // CO2
    if (co2_switch.is_set() && !new_co2_state) {
        // We are turning CO2 off...
        // Lock co2 switch so it can't be turned on too soon
        co2_deciseconds_until_can_turn_on = AK_CO2_OFF_HOURS_BEFORE_UNLOCKED * 60L * 60L * 10L;
    }
    co2_switch.set(new_co2_state);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Uptime

GLOBAL$() {
    STATIC_VAR$(u32 uptime_deciseconds);
}

X_EVERY_DECISECOND$(uptime_ticker) {
    uptime_deciseconds += 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Led pins

X_GPIO_OUTPUT$(blue_led, B7);


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Watchdog

X_WATCHDOG$(8s);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Activity indication

// This piece of code will be executed in akat event/thread loop every 1/10 second.
// We use to turn the blue led ON and OFF
X_EVERY_DECISECOND$(activity_led) {
    STATIC_VAR$(u8 state);

    blue_led.set(state);
    state = !state;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// DS18B20

X_DS18B20$(ds18b20_aqua, A0);
X_DS18B20$(ds18b20_case, A1);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// ADC

X_INIT$(init_adc) {
    // We just use default ADC channel (ADC0 marked as 'A0' on the PCB)
    // No need to change or setup that. But we must select reference voltage for ADC.
    // We use AVCC which is connected to VCC on the board.
    ADMUX = H(REFS0);

    // Setup prescaler.
    // Slower we do measurement, better are results.
    // Higher prescaler means lower frequency of ADC.
    // Highest prescaler is 128. 16Mhz / 128 = 125khz.
    // ADPS - Prescaler Selection. ADEN - enables ADC.
    ADCSRA = H(ADPS2) | H(ADPS1) | H(ADPS0) | H(ADEN);

    // Immediately start AD-conversion for PH-Meter
    ADCSRA |= H(ADSC);
};

GLOBAL$() {
    STATIC_VAR$(u24 ph_adc_accum);
    STATIC_VAR$(u16 ph_adc_accum_samples);
};

RUNNABLE$(adc_runnable) {
    if (!(ADCSRA & H(ADSC))) {
        // No conversions are in progress now, read current value and start a new conversion

        // First store current value into a temporary variable
        u16 current_adc = ADC;

        // Start new conversion
        ADCSRA |= H(ADSC);

        // Add current value into accumulator
        ph_adc_accum += current_adc;
        ph_adc_accum_samples += 1;
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// USART1 - MH-Z19 CO2 Module

// Add for debug: debug = add_debug_byte
X_MHZ19$(co2, uart = 1, use_abc = 0);

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// USART0 - Serial interface over USB Connection

X_INIT$(usart0_init) {
    // Set baud rate
    const u16 ubrr = akat_cpu_freq_hz() / (AK_USART0_BAUD_RATE * 8L) - 1;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr % 256;
    UCSR0A = H(U2X0);

    // Set frame format
    UCSR0C = AK_USART0_FRAME_FORMAT;

    // Enable transmitter, receiver and interrupt for receiver (interrupt for 'byte is received')
    UCSR0B = H(TXEN0) | H(RXEN0) | H(RXCIE0);
}

// ----------------------------------------------------------------
// USART0(USB): Interrupt handler for 'byte is received' event..

GLOBAL$() {
    STATIC_VAR$(volatile u8 usart0_rx_bytes_buf[AK_USART0_RX_BUF_SIZE], initial = {});
    STATIC_VAR$(volatile u8 usart0_rx_overflow_count);
    STATIC_VAR$(volatile u8 usart0_rx_next_empty_idx);
    STATIC_VAR$(volatile u8 usart0_rx_next_read_idx);
}

ISR(USART0_RX_vect) {
    u8 b = UDR0; // we must read here, no matter what, to clear interrupt flag

    u8 new_next_empty_idx = (usart0_rx_next_empty_idx + AKAT_ONE) & (AK_USART0_RX_BUF_SIZE - 1);
    if (new_next_empty_idx == usart0_rx_next_read_idx) {
        usart0_rx_overflow_count += AKAT_ONE;
        // Don't let it overflow!
        if (!usart0_rx_overflow_count) {
            usart0_rx_overflow_count -= AKAT_ONE;
        }
    } else {
        usart0_rx_bytes_buf[usart0_rx_next_empty_idx] = b;
        usart0_rx_next_empty_idx = new_next_empty_idx;
    }
}

// ----------------------------------------------------------------
// USART0(USB): This thread continuously writes current status into USART0

// NOTE: Just replace state_type to u16 if we ran out of state space..
THREAD$(usart0_writer, state_type = u8) {
    // ---- All variable in the thread must be static (green threads requirement)
    STATIC_VAR$(u8 crc);
    STATIC_VAR$(u8 byte_to_send);
    STATIC_VAR$(u8 u8_to_format_and_send);
    STATIC_VAR$(u16 u16_to_format_and_send);
    STATIC_VAR$(u32 u32_to_format_and_send);
    
    STATIC_VAR$(u24 __ph_adc_accum);
    STATIC_VAR$(u16 __ph_adc_accum_samples);

    // ---- Subroutines can yield unlike functions

    SUB$(send_byte) {
        // Wait until USART0 is ready to transmit next byte
        // from 'byte_to_send';
        WAIT_UNTIL$(UCSR0A & H(UDRE0), unlikely);
        UDR0 = byte_to_send;
        crc = akat_crc_add(crc, byte_to_send);
    }

    SUB$(format_and_send_u8) {
        if (u8_to_format_and_send) {
            u8 h = u8_to_format_and_send / 16;
            if (h) {
                byte_to_send = HEX[h]; CALL$(send_byte);
            }

            u8 i = u8_to_format_and_send & 15;
            byte_to_send = HEX[i]; CALL$(send_byte);
        }
    }

    SUB$(format_and_send_u16) {
        u8_to_format_and_send = (u8)(u16_to_format_and_send / 256);
        if (u8_to_format_and_send) {
            CALL$(format_and_send_u8);

            u8_to_format_and_send = (u8)u16_to_format_and_send;
            byte_to_send = HEX[u8_to_format_and_send / 16]; CALL$(send_byte);
            byte_to_send = HEX[u8_to_format_and_send & 15]; CALL$(send_byte);
        } else {
            u8_to_format_and_send = (u8)u16_to_format_and_send;
            CALL$(format_and_send_u8);
        }
    }

    SUB$(format_and_send_u32) {
        u16_to_format_and_send = (u16)(u32_to_format_and_send >> 16);
        if (u16_to_format_and_send) {
            CALL$(format_and_send_u16);

            u16_to_format_and_send = (u16)u32_to_format_and_send;
            u8_to_format_and_send = (u8)(u16_to_format_and_send / 256);
            byte_to_send = HEX[u8_to_format_and_send / 16]; CALL$(send_byte);
            byte_to_send = HEX[u8_to_format_and_send & 15]; CALL$(send_byte);
            u8_to_format_and_send = (u8)u16_to_format_and_send;
            byte_to_send = HEX[u8_to_format_and_send / 16]; CALL$(send_byte);
            byte_to_send = HEX[u8_to_format_and_send & 15]; CALL$(send_byte);
        } else {
            u16_to_format_and_send = (u16)u32_to_format_and_send;
            CALL$(format_and_send_u16);
        }
    }

    // ---- Macro that writes the given status into UART
    // We also write some humand readable description of the protocol
    // also stuff to distinguish protocol versions and generate typescript parser code

    DEFINE_MACRO$(WRITE_STATUS, required_args = ["name", "id"], keep_rest_as_is = True) {
        byte_to_send = ' '; CALL$(send_byte);
        byte_to_send = '${id}'; CALL$(send_byte);

        % for arg in rest:
            /*
              COMMPROTO: ${id}${loop.index+1}: ${name.replace('"', "")}: ${arg}
              TS_PROTO_TYPE: "${arg}": number,
              TS_PROTO_ASSIGN: "${arg}": vals["${id}${loop.index+1}"],
            */
            <% [argt, argn] = arg.split(" ", 1) %>
            ${argt}_to_format_and_send = ${argn}; CALL$(format_and_send_${argt});
            % if not loop.last:
                byte_to_send = ','; CALL$(send_byte);
            % endif
        % endfor
    }

    // - - - - - - - - - - -
    // Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
    while(1) {
        // ---- - - - - -- - - - - - - -
        // Write debug if there is some
        if (debug_next_empty_idx != debug_next_read_idx) {
            while (debug_next_empty_idx != debug_next_read_idx) {
                byte_to_send = '>'; CALL$(send_byte);

                // Read byte first, then increment idx!
                u8_to_format_and_send = debug_bytes_buf[debug_next_read_idx];
                debug_next_read_idx = (debug_next_read_idx + 1) & (AK_DEBUG_BUF_SIZE - 1);

                CALL$(format_and_send_u8);
            }

            byte_to_send = '\r'; CALL$(send_byte);
            byte_to_send = '\n'; CALL$(send_byte);
        }


        // ----  - - - - -- - - - - -

        crc = 0;

        // WRITE_STATUS(name for documentation, 1-character id for protocol, type1 val1, type2 val2, ...)

        WRITE_STATUS$(Misc,
                      A,
                      u32 uptime_deciseconds,
                      u8 debug_overflow_count,
                      u8 usart0_rx_overflow_count,
                      u32 main_loop_iterations_in_last_decisecond,
                      u32 ((u32)last_drift_of_clock_deciseconds_since_midnight),
                      u32 clock_corrections_since_protection_stat_reset,
                      u32 clock_deciseconds_since_midnight);

        WRITE_STATUS$("Aquarium temperature sensor",
                      B,
                      u8 ds18b20_aqua.get_crc_errors(),
                      u8 ds18b20_aqua.get_disconnects(),
                      u16 ds18b20_aqua.get_temperatureX16(),
                      u8 ds18b20_aqua.get_update_id(),
                      u8 ds18b20_aqua.get_updated_deciseconds_ago());

        WRITE_STATUS$("Case temperature sensor",
                      C,
                      u8 ds18b20_case.get_crc_errors(),
                      u8 ds18b20_case.get_disconnects(),
                      u16 ds18b20_case.get_temperatureX16(),
                      u8 ds18b20_case.get_update_id(),
                      u8 ds18b20_case.get_updated_deciseconds_ago());

        WRITE_STATUS$("CO2 sensor",
                      D,
                      u8 co2.get_rx_overflow_count(),
                      u8 co2.get_crc_errors(),
                      u16 co2.get_abc_setups(),
                      u16 co2.get_raw_concentration(),
                      u16 co2.get_clamped_concentration(),
                      u16 co2.get_concentration(),
                      u8 co2.get_temperature(),
                      u8 co2.get_s(),
                      u16 co2.get_u(),
                      u8 co2.get_update_id(),
                      u8 co2.get_updated_deciseconds_ago(),
                      u8 co2_switch.is_set() ? 1 : 0,
                      u8 co2_calculated_day ? 1 : 0,
                      u8 co2_force_off.is_set() ? 1 : 0,
                      u8 required_co2_switch_state.is_set() ? 1 : 0,
                      u32 co2_deciseconds_until_can_turn_on);

        WRITE_STATUS$("Light",
                      E,
                      u8 day_light_switch.is_set() ? 1 : 0,
                      u8 night_light_switch.is_set() ? 1 : 0,
                      u8 day_light_forced.is_set() ? 1 : 0,
                      u8 night_light_forced.is_set() ? 1 : 0,
                      u8 light_forces_since_protection_stat_reset);

        // Special handling for ph meter ADC result.
        // Remember values before writing and then set current accum values to zero
        // This is because of this thread might YIELD and we don't want our stuff to be
        // messes up in the middle of the process.
        __ph_adc_accum = ph_adc_accum;
        __ph_adc_accum_samples = ph_adc_accum_samples;

        // Set to zero to start a new oversampling batch
        ph_adc_accum = 0;
        ph_adc_accum_samples = 0;

        // Write Voltage status... this might YIELD
        WRITE_STATUS$("PH Voltage",
                      F,
                      u32 __ph_adc_accum,
                      u16 __ph_adc_accum_samples);

        // Protocol version
        byte_to_send = ' '; CALL$(send_byte);
        u8_to_format_and_send = AK_PROTOCOL_VERSION; CALL$(format_and_send_u8);

        // Done writing status, send: CRC\r\n
        byte_to_send = ' '; CALL$(send_byte);
        u8_to_format_and_send = crc; CALL$(format_and_send_u8);

        // Newline
        byte_to_send = '\r'; CALL$(send_byte);
        byte_to_send = '\n'; CALL$(send_byte);
    }
}

// ---------------------------------------------------------------------------------
// USART0(USB): This thread processes input from usart0_rx_bytes_buf that gets populated in ISR

THREAD$(usart0_reader) {
    // ---- all variable in the thread must be static (green threads requirement)
    STATIC_VAR$(u8 command_code);
    STATIC_VAR$(u8 command_arg);

    // Subroutine that reads a command from the input
    // Command is expected to be in the format as one printed out by 'send_status'
    // Command end ups in 'command_code' variable and optional
    // arguments end ups in 'command_arg'. If commands comes without argument, then
    // we assume it is 0 by convention.
    SUB$(read_command) {
        STATIC_VAR$(u8 dequeued_byte);
        STATIC_VAR$(u8 command_arg_copy);

        // Gets byte from usart0_rx_bytes_buf buffer.
        SUB$(dequeue_byte) {
            // Wait until there is something to read
            WAIT_UNTIL$(usart0_rx_next_empty_idx != usart0_rx_next_read_idx, unlikely);

            // Read byte first, then increment idx!
            dequeued_byte = usart0_rx_bytes_buf[usart0_rx_next_read_idx];
            usart0_rx_next_read_idx = (usart0_rx_next_read_idx + 1) & (AK_USART0_RX_BUF_SIZE - 1);
        }

        // Read arg into command_arg, leaves byte after arg in the dequeued_byte variable
        // so the caller must process it as well upon return!
        SUB$(read_arg_and_dequeue) {
            command_arg = 0;
            CALL$(dequeue_byte);
            if (dequeued_byte >= '0' && dequeued_byte <= '9') {
                command_arg = dequeued_byte - '0';
                CALL$(dequeue_byte);
                if (dequeued_byte >= '0' && dequeued_byte <= '9') {
                    command_arg = command_arg * 10 + (dequeued_byte - '0');
                    CALL$(dequeue_byte);
                    if (dequeued_byte >= '0' && dequeued_byte <= '9') {
                        if (command_arg < 25 || (command_arg == 25 && dequeued_byte <= '5')) {
                            command_arg = command_arg * 10 + (dequeued_byte - '0');
                            CALL$(dequeue_byte);
                        }
                    }
                }
            }
        }

    command_reading_start:
        // Read opening bracket
        CALL$(dequeue_byte);
        if (dequeued_byte != '<') {
            goto command_reading_start;
        }

        // Read command code
        // Verify that code is really a code letter
        CALL$(dequeue_byte);
        if (dequeued_byte < 'A' || dequeued_byte > 'Z') {
            goto command_reading_start;
        }
        command_code = dequeued_byte;

        // Read arg and save it as copy, note that read_arg aborts
        // when it either read fourth character in a row or a non digit character
        // so we have to process it (dequeued_byte) when the call to read_arg returns.
        // Verify that stuff that comes after the arg is a command code again!
        CALL$(read_arg_and_dequeue);
        if (dequeued_byte != command_code) {
            goto command_reading_start;
        }
        command_arg_copy = command_arg;

        // Read command arg once again (it comes after a copy of command code which we already verified)
        // We also verify that there is an > character right after the arg
        // And of course we verify that arg matches the copy we read before.
        CALL$(read_arg_and_dequeue);
        if (dequeued_byte != '>' || command_arg_copy != command_arg) {
            goto command_reading_start;
        }
    }

    // - - - - - - - - - - -
    // Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
    while(1) {
        // Read command and put results into 'command_code' and 'command_arg'.
        CALL$(read_command);

        switch(command_code) {
        case 'F':
            co2_force_off.set(AKAT_ONE);
            break;

        case 'G':
            update_co2_switch_state(command_arg);
            break;

        case 'L':
            force_light(command_arg);
            break;

        case 'A':
            // Least significant clock byte
            received_clock0 = command_arg;
            break;

        case 'B':
            received_clock1 = command_arg;
            break;

        case 'C':
            // Most significant clock byte. This one is supposed to be received LAST.
            // ('A' and 'B') must be already here.
            received_clock2 = command_arg;
            break;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Main

X_MAIN$() {
    // Enable interrupts
    sei();
}
