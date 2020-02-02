;

// NOTE: Sometimes it's nice to try to see which one is best to have some not low, must try some combinations
// USE_REG$(global variable name);
// USE_REG$(global variable name, low);

// TUNE_FUNCTION$(function name, pure, no_inline);

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#define i8 int8_t
#define i16 int16_t
#define i32 int32_t

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define H(v)                 (1 << (v))
#define L(v)                 (0)

#define AKAT_FORCE_INLINE    __attribute__((always_inline)) inline
#define AKAT_NO_INLINE       __attribute__((noinline))
#define AKAT_UNUSED          __attribute__((unused))
#define AKAT_NO_RETURN       __ATTR_NORETURN__
#define AKAT_CONST           __ATTR_CONST__
#define AKAT_PURE            __ATTR_PURE__
#define AKAT_ERROR(msg)      __attribute__((error(msg))) extern void

#define AKAT_CONCAT(a, b)     a##b
#define AKAT_FORCE_CONCAT(a, b)     AKAT_CONCAT(a, b)

#define AKAT_HOT_CODE        AKAT_FORCE_CONCAT(akat_hot_code__, __COUNTER__): __attribute__((hot, unused));
#define AKAT_COLD_CODE       AKAT_FORCE_CONCAT(akat_cold_code__, __COUNTER__): __attribute__((cold, unused));

#define AKAT_FLUSH_REG_VAR(vvv)     asm volatile ("" : "=r" (vvv));

#define AKAT_COROUTINE_S_START   0
#define AKAT_COROUTINE_S_END     255

static AKAT_FORCE_INLINE AKAT_CONST uint32_t akat_cpu_freq_hz();

register u8 __akat_one__ asm ("r2");

;
;


// To prevent assignment
#define AKAT_ONE  __akat_one__

#define AKAT_TRUE   AKAT_ONE
#define AKAT_FALSE  0

// ============================================================================================================================
// Compatibility

#ifndef TIMSK1
#define TIMSK1 TIMSK
#endif

// ============================================================================================================================
// DELAY

// Delay. Delay function is non atomic!
// Routines are borrowed from avr-lib
__attribute__((error("akat_delay_us must be used with -O compiler flag and constant argument!")))
extern void akat_delay_us_error_nc__();

__attribute__((error("akat_delay_us can't perform such a small delay!")))
extern void akat_delay_us_error_delay__();

__attribute__((error("akat_delay_us can't perform such a long delay!")))
extern void akat_delay_us_error_bdelay__();

static AKAT_FORCE_INLINE void akat_delay_us(uint32_t us) {
    if (!__builtin_constant_p(us)) {
        akat_delay_us_error_nc__ ();
    }

    uint64_t cycles = (uint64_t)us * (uint64_t)akat_cpu_freq_hz () / (uint64_t)1000000L;

    if (cycles / 3 == 0) {
        akat_delay_us_error_delay__ ();
    } else if (cycles / 3 < 256) {
        uint8_t __count = cycles / 3;
        __asm__ volatile (
            "1: dec %0" "\n\t"
            "brne 1b"
            : "=r" (__count)
            : "0" (__count)
        );
    } else if (cycles / 4 > 65535) {
        akat_delay_us_error_bdelay__ ();
    } else {
        uint16_t __count = cycles / 4;
        __asm__ volatile (
            "1: sbiw %0,1" "\n\t"
            "brne 1b"
            : "=w" (__count)
            : "0" (__count)
        );
    }
}

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2019 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

static AKAT_PURE u8 akat_crc_add(u8 const crc, u8 const byte);
static u8 akat_crc_add_bytes(u8 const crc, u8 const *bytes, const u8 size);

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

#define AKAT_BCD_GET_L(x)     ((x) & 15)
#define AKAT_BCD_GET_H(x)     (((x) / 16))
#define AKAT_BCD(h, l)        (((h) * 16) + (l))

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

#define AKAT_X_BUTTON_CHECKS 255

typedef void (*akat_x_button_cbk_t)();

typedef enum {AKAT_X_BUTTON_ACTION_NOTHING = 0, AKAT_X_BUTTON_ACTION_KEYPRESS = 1, AKAT_X_BUTTON_ACTION_KEYRELEASE = 2} akat_x_button_action_t;

typedef struct {
    uint8_t awaiting_key_press;
    uint8_t checks_left;
} akat_x_button_state_t;

static AKAT_UNUSED akat_x_button_action_t akat_x_button_handle_pin_state(akat_x_button_state_t * const state, uint8_t const pin_state);

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

//
//     a
//    ----
//   |    | b
//  f|    |
//   |-g--|
//   |    | c
//  e|    |
//    ----
//     d        : h
//


//                                    hgfedcba
#define AKAT_X_TM1637_C_0           0b00111111
#define AKAT_X_TM1637_C_1           0b00000110
#define AKAT_X_TM1637_C_2           0b01011011
#define AKAT_X_TM1637_C_3           0b01001111
#define AKAT_X_TM1637_C_4           0b01100110
#define AKAT_X_TM1637_C_5           0b01101101
#define AKAT_X_TM1637_C_6           0b01111101
#define AKAT_X_TM1637_C_7           0b00000111
#define AKAT_X_TM1637_C_8           0b01111111
#define AKAT_X_TM1637_C_9           0b01101111

#define AKAT_X_TM1637_C_D           0b00111111
#define AKAT_X_TM1637_C_o           0b01011100
#define AKAT_X_TM1637_C_n           0b01010100
#define AKAT_X_TM1637_C_E           0b01111001

#define AKAT_X_TM1637_COLON_MASK    0b10000000

static AKAT_PURE u8 akat_x_tm1637_encode_digit(u8 const  digit, u8 const  colon);

typedef enum {AKAT_X_TM1637_POS_1 = 0, AKAT_X_TM1637_POS_2 = 1, AKAT_X_TM1637_POS_3 = 2, AKAT_X_TM1637_POS_4 = 3} akat_x_tm1637_pos_t;

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

typedef enum {
    AKAT_X_TIMESTAMP_LEVEL_DECISECOND,
    AKAT_X_TIMESTAMP_LEVEL_SECOND,
    AKAT_X_TIMESTAMP_LEVEL_MINUTE,
    AKAT_X_TIMESTAMP_LEVEL_HOUR
} akat_x_timestamp_level_t;

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

typedef struct {
    uint8_t const cs;
    uint8_t const ocr;
    uint8_t const deciseconds;
} akat_x_buzzer_sound_t;

typedef void (*akat_x_buzzer_finish_cbk_t)(u8 interrupted);

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

static AKAT_PURE AKAT_UNUSED u8 akat_bcd_inc(u8 bcd) {
    if (AKAT_BCD_GET_L(bcd) == 9) {
        bcd += 16 - 9;
    } else {
        bcd++;
    }

    return bcd;
}

;




static AKAT_PURE AKAT_UNUSED u8 akat_bcd_dec(u8 bcd) {
    if (AKAT_BCD_GET_L(bcd)) {
        bcd--;
    } else {
        bcd += -16 + 9;
    }

    return bcd;
}

;




///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2019 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

static AKAT_UNUSED AKAT_PURE u8 akat_crc_add(u8 const orig_crc, u8 const orig_byte) {
    u8 crc = orig_crc;
    u8 byte = orig_byte;

    for (u8 j = 0; j < 8; j++) {
        u8 m = (crc ^ byte) & AKAT_ONE;
        crc >>= 1;

        if (m) {
            crc ^= 0x8C;
        }

        byte >>= 1;
    }

    return crc;
}

static AKAT_UNUSED u8 akat_crc_add_bytes(u8 const orig_crc, u8 const *bytes, const u8 size) {
    u8 crc = orig_crc;

    for (u8 i = 0; i < size; i++) {
        crc = akat_crc_add(crc, bytes[i]);
    }

    return crc;
}

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

static AKAT_UNUSED akat_x_button_action_t akat_x_button_handle_pin_state(akat_x_button_state_t * const state, uint8_t const pin_state) {
    akat_x_button_action_t action = AKAT_X_BUTTON_ACTION_NOTHING;

    if (state->awaiting_key_press) {
        AKAT_HOT_CODE; // Usually we are awaiting a key press

        if (pin_state) {
            AKAT_HOT_CODE; // Often pin is high, it means that key is not pressed, reset counter
            state->checks_left = AKAT_X_BUTTON_CHECKS;
        } else {
            AKAT_COLD_CODE; // Sometimes pin is low, it means that key is pressed

            if (state->checks_left) {
                AKAT_HOT_CODE; // Usually we need to do more checks
                state->checks_left--;
            } else {
                AKAT_COLD_CODE; // Sometimes we find out that key is pressed and stable enough
                // Notify about key-press event
                action = AKAT_X_BUTTON_ACTION_KEYPRESS;
                // Wait for key-release event
                state->awaiting_key_press = 0;
                state->checks_left = AKAT_X_BUTTON_CHECKS;
            }
        }
    } else {
        AKAT_COLD_CODE; // Sometimes we are awaiting for key to be released

        if (pin_state) {
            AKAT_COLD_CODE; // After a long keypress, pin can be high, it means that key is released

            if (state->checks_left) {
                AKAT_HOT_CODE; // Often we have to check again to make sure that state is stable, not bouncing
                state->checks_left--;
            } else {
                AKAT_COLD_CODE; // When we checked enough times, wait for key-press event
                // Notify about key-press event
                action = AKAT_X_BUTTON_ACTION_KEYRELEASE;
                // Wait for key press
                state->awaiting_key_press = AKAT_ONE;
                state->checks_left = AKAT_X_BUTTON_CHECKS;
            }
        } else {
            AKAT_HOT_CODE; // Pin is low, it means that key is pressed
            state->checks_left = AKAT_X_BUTTON_CHECKS;
        }
    }

    return action;
}

///////////////////////////////////////////////////////////////////
// Useful functions for rapid development for AVR microcontrollers.
// 2017 (C) Akshaal, Apache License
///////////////////////////////////////////////////////////////////

#include <avr/pgmspace.h>

static PROGMEM AKAT_UNUSED u8 const akat_x_tm1637_digits_map[] = {
    AKAT_X_TM1637_C_0,
    AKAT_X_TM1637_C_1,
    AKAT_X_TM1637_C_2,
    AKAT_X_TM1637_C_3,
    AKAT_X_TM1637_C_4,
    AKAT_X_TM1637_C_5,
    AKAT_X_TM1637_C_6,
    AKAT_X_TM1637_C_7,
    AKAT_X_TM1637_C_8,
    AKAT_X_TM1637_C_9
};

static AKAT_UNUSED AKAT_PURE u8 akat_x_tm1637_encode_digit(u8 const digit, u8 const colon) {
    return pgm_read_byte(akat_x_tm1637_digits_map + digit) | (colon ? AKAT_X_TM1637_COLON_MASK : 0);
}

#include <avr/interrupt.h>
#include <avr/io.h>

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

// 16Mhz, that's external oscillator on Mega 2560.
// This doesn't configure it here, it just tells to our build system
// what we is actually using! Configuration is done using fuses (see flash-avr-fuses).
// Actual value might be a different one, one can actually measure it to provide
// some kind of accuracy (calibration) if needed.
static AKAT_FORCE_INLINE AKAT_CONST uint32_t akat_cpu_freq_hz() {
    return 16000000;
}
;


static const char HEX[16] = "0123456789abcdef";


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// PINS

// For safety reasons we set unused pins to read with pull up

typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G5_unused__port_t;

extern G5_unused__port_t const G5_unused__port;

static AKAT_FORCE_INLINE void G5_unused__port__set__impl(u8 state) {
#define set__impl G5_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 5;  //Set PORTG of G5 to 1
    } else {
        PORTG &= ~(1 << 5);  //Set PORTG of G5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G5_unused__port__is_set__impl() {
#define is_set__impl G5_unused__port__is_set__impl
#define set__impl G5_unused__port__set__impl
    return PORTG & (1 << 5);  //Get value of PORTG for G5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G5_unused__port__is_set__impl
#define set__impl G5_unused__port__set__impl

G5_unused__port_t const G5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G5_unused__port__is_set__impl
#define set__impl G5_unused__port__set__impl


;

#define is_set__impl G5_unused__port__is_set__impl
#define set__impl G5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G5_unused__ddr_t;

extern G5_unused__ddr_t const G5_unused__ddr;

static AKAT_FORCE_INLINE void G5_unused__ddr__set__impl(u8 state) {
#define set__impl G5_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 5;  //Set DDRG of G5 to 1
    } else {
        DDRG &= ~(1 << 5);  //Set DDRG of G5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G5_unused__ddr__is_set__impl() {
#define is_set__impl G5_unused__ddr__is_set__impl
#define set__impl G5_unused__ddr__set__impl
    return DDRG & (1 << 5);  //Get value of DDRG for G5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G5_unused__ddr__is_set__impl
#define set__impl G5_unused__ddr__set__impl

G5_unused__ddr_t const G5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G5_unused__ddr__is_set__impl
#define set__impl G5_unused__ddr__set__impl


;

#define is_set__impl G5_unused__ddr__is_set__impl
#define set__impl G5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G5_unused__pin_t;

extern G5_unused__pin_t const G5_unused__pin;

static AKAT_FORCE_INLINE void G5_unused__pin__set__impl(u8 state) {
#define set__impl G5_unused__pin__set__impl

    if (state) {
        PING |= 1 << 5;  //Set PING of G5 to 1
    } else {
        PING &= ~(1 << 5);  //Set PING of G5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G5_unused__pin__is_set__impl() {
#define is_set__impl G5_unused__pin__is_set__impl
#define set__impl G5_unused__pin__set__impl
    return PING & (1 << 5);  //Get value of PING for G5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G5_unused__pin__is_set__impl
#define set__impl G5_unused__pin__set__impl

G5_unused__pin_t const G5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G5_unused__pin__is_set__impl
#define set__impl G5_unused__pin__set__impl


;

#define is_set__impl G5_unused__pin__is_set__impl
#define set__impl G5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G5_unused__init() {
    G5_unused__ddr.set(0);
    G5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G5_unused_t;

extern G5_unused_t const G5_unused;

static AKAT_FORCE_INLINE u8 G5_unused__is_set__impl() {
#define is_set__impl G5_unused__is_set__impl
    return G5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G5_unused__is_set__impl

G5_unused_t const G5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G5_unused__is_set__impl


;

#define is_set__impl G5_unused__is_set__impl




#undef is_set__impl
;



;
; // 1    PG5 ( OC0B ) Digital pin 4 (PWM)
// USART0 / USB ..... 2    PE0 ( RXD0/PCINT8 ) Digital pin 0 (RX0)
// USART0 / USB ..... 3    PE1 ( TXD0 ) Digital pin 1 (TX0)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E2_unused__port_t;

extern E2_unused__port_t const E2_unused__port;

static AKAT_FORCE_INLINE void E2_unused__port__set__impl(u8 state) {
#define set__impl E2_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 2;  //Set PORTE of E2 to 1
    } else {
        PORTE &= ~(1 << 2);  //Set PORTE of E2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E2_unused__port__is_set__impl() {
#define is_set__impl E2_unused__port__is_set__impl
#define set__impl E2_unused__port__set__impl
    return PORTE & (1 << 2);  //Get value of PORTE for E2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E2_unused__port__is_set__impl
#define set__impl E2_unused__port__set__impl

E2_unused__port_t const E2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E2_unused__port__is_set__impl
#define set__impl E2_unused__port__set__impl


;

#define is_set__impl E2_unused__port__is_set__impl
#define set__impl E2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E2_unused__ddr_t;

extern E2_unused__ddr_t const E2_unused__ddr;

static AKAT_FORCE_INLINE void E2_unused__ddr__set__impl(u8 state) {
#define set__impl E2_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 2;  //Set DDRE of E2 to 1
    } else {
        DDRE &= ~(1 << 2);  //Set DDRE of E2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E2_unused__ddr__is_set__impl() {
#define is_set__impl E2_unused__ddr__is_set__impl
#define set__impl E2_unused__ddr__set__impl
    return DDRE & (1 << 2);  //Get value of DDRE for E2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E2_unused__ddr__is_set__impl
#define set__impl E2_unused__ddr__set__impl

E2_unused__ddr_t const E2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E2_unused__ddr__is_set__impl
#define set__impl E2_unused__ddr__set__impl


;

#define is_set__impl E2_unused__ddr__is_set__impl
#define set__impl E2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E2_unused__pin_t;

extern E2_unused__pin_t const E2_unused__pin;

static AKAT_FORCE_INLINE void E2_unused__pin__set__impl(u8 state) {
#define set__impl E2_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 2;  //Set PINE of E2 to 1
    } else {
        PINE &= ~(1 << 2);  //Set PINE of E2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E2_unused__pin__is_set__impl() {
#define is_set__impl E2_unused__pin__is_set__impl
#define set__impl E2_unused__pin__set__impl
    return PINE & (1 << 2);  //Get value of PINE for E2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E2_unused__pin__is_set__impl
#define set__impl E2_unused__pin__set__impl

E2_unused__pin_t const E2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E2_unused__pin__is_set__impl
#define set__impl E2_unused__pin__set__impl


;

#define is_set__impl E2_unused__pin__is_set__impl
#define set__impl E2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E2_unused__init() {
    E2_unused__ddr.set(0);
    E2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E2_unused_t;

extern E2_unused_t const E2_unused;

static AKAT_FORCE_INLINE u8 E2_unused__is_set__impl() {
#define is_set__impl E2_unused__is_set__impl
    return E2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E2_unused__is_set__impl

E2_unused_t const E2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E2_unused__is_set__impl


;

#define is_set__impl E2_unused__is_set__impl




#undef is_set__impl
;



;
; // 4    PE2 ( XCK0/AIN0 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E3_unused__port_t;

extern E3_unused__port_t const E3_unused__port;

static AKAT_FORCE_INLINE void E3_unused__port__set__impl(u8 state) {
#define set__impl E3_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 3;  //Set PORTE of E3 to 1
    } else {
        PORTE &= ~(1 << 3);  //Set PORTE of E3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E3_unused__port__is_set__impl() {
#define is_set__impl E3_unused__port__is_set__impl
#define set__impl E3_unused__port__set__impl
    return PORTE & (1 << 3);  //Get value of PORTE for E3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E3_unused__port__is_set__impl
#define set__impl E3_unused__port__set__impl

E3_unused__port_t const E3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E3_unused__port__is_set__impl
#define set__impl E3_unused__port__set__impl


;

#define is_set__impl E3_unused__port__is_set__impl
#define set__impl E3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E3_unused__ddr_t;

extern E3_unused__ddr_t const E3_unused__ddr;

static AKAT_FORCE_INLINE void E3_unused__ddr__set__impl(u8 state) {
#define set__impl E3_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 3;  //Set DDRE of E3 to 1
    } else {
        DDRE &= ~(1 << 3);  //Set DDRE of E3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E3_unused__ddr__is_set__impl() {
#define is_set__impl E3_unused__ddr__is_set__impl
#define set__impl E3_unused__ddr__set__impl
    return DDRE & (1 << 3);  //Get value of DDRE for E3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E3_unused__ddr__is_set__impl
#define set__impl E3_unused__ddr__set__impl

E3_unused__ddr_t const E3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E3_unused__ddr__is_set__impl
#define set__impl E3_unused__ddr__set__impl


;

#define is_set__impl E3_unused__ddr__is_set__impl
#define set__impl E3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E3_unused__pin_t;

extern E3_unused__pin_t const E3_unused__pin;

static AKAT_FORCE_INLINE void E3_unused__pin__set__impl(u8 state) {
#define set__impl E3_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 3;  //Set PINE of E3 to 1
    } else {
        PINE &= ~(1 << 3);  //Set PINE of E3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E3_unused__pin__is_set__impl() {
#define is_set__impl E3_unused__pin__is_set__impl
#define set__impl E3_unused__pin__set__impl
    return PINE & (1 << 3);  //Get value of PINE for E3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E3_unused__pin__is_set__impl
#define set__impl E3_unused__pin__set__impl

E3_unused__pin_t const E3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E3_unused__pin__is_set__impl
#define set__impl E3_unused__pin__set__impl


;

#define is_set__impl E3_unused__pin__is_set__impl
#define set__impl E3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E3_unused__init() {
    E3_unused__ddr.set(0);
    E3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E3_unused_t;

extern E3_unused_t const E3_unused;

static AKAT_FORCE_INLINE u8 E3_unused__is_set__impl() {
#define is_set__impl E3_unused__is_set__impl
    return E3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E3_unused__is_set__impl

E3_unused_t const E3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E3_unused__is_set__impl


;

#define is_set__impl E3_unused__is_set__impl




#undef is_set__impl
;



;
; // 5    PE3 ( OC3A/AIN1 ) Digital pin 5 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E4_unused__port_t;

extern E4_unused__port_t const E4_unused__port;

static AKAT_FORCE_INLINE void E4_unused__port__set__impl(u8 state) {
#define set__impl E4_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 4;  //Set PORTE of E4 to 1
    } else {
        PORTE &= ~(1 << 4);  //Set PORTE of E4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E4_unused__port__is_set__impl() {
#define is_set__impl E4_unused__port__is_set__impl
#define set__impl E4_unused__port__set__impl
    return PORTE & (1 << 4);  //Get value of PORTE for E4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E4_unused__port__is_set__impl
#define set__impl E4_unused__port__set__impl

E4_unused__port_t const E4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E4_unused__port__is_set__impl
#define set__impl E4_unused__port__set__impl


;

#define is_set__impl E4_unused__port__is_set__impl
#define set__impl E4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E4_unused__ddr_t;

extern E4_unused__ddr_t const E4_unused__ddr;

static AKAT_FORCE_INLINE void E4_unused__ddr__set__impl(u8 state) {
#define set__impl E4_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 4;  //Set DDRE of E4 to 1
    } else {
        DDRE &= ~(1 << 4);  //Set DDRE of E4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E4_unused__ddr__is_set__impl() {
#define is_set__impl E4_unused__ddr__is_set__impl
#define set__impl E4_unused__ddr__set__impl
    return DDRE & (1 << 4);  //Get value of DDRE for E4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E4_unused__ddr__is_set__impl
#define set__impl E4_unused__ddr__set__impl

E4_unused__ddr_t const E4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E4_unused__ddr__is_set__impl
#define set__impl E4_unused__ddr__set__impl


;

#define is_set__impl E4_unused__ddr__is_set__impl
#define set__impl E4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E4_unused__pin_t;

extern E4_unused__pin_t const E4_unused__pin;

static AKAT_FORCE_INLINE void E4_unused__pin__set__impl(u8 state) {
#define set__impl E4_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 4;  //Set PINE of E4 to 1
    } else {
        PINE &= ~(1 << 4);  //Set PINE of E4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E4_unused__pin__is_set__impl() {
#define is_set__impl E4_unused__pin__is_set__impl
#define set__impl E4_unused__pin__set__impl
    return PINE & (1 << 4);  //Get value of PINE for E4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E4_unused__pin__is_set__impl
#define set__impl E4_unused__pin__set__impl

E4_unused__pin_t const E4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E4_unused__pin__is_set__impl
#define set__impl E4_unused__pin__set__impl


;

#define is_set__impl E4_unused__pin__is_set__impl
#define set__impl E4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E4_unused__init() {
    E4_unused__ddr.set(0);
    E4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E4_unused_t;

extern E4_unused_t const E4_unused;

static AKAT_FORCE_INLINE u8 E4_unused__is_set__impl() {
#define is_set__impl E4_unused__is_set__impl
    return E4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E4_unused__is_set__impl

E4_unused_t const E4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E4_unused__is_set__impl


;

#define is_set__impl E4_unused__is_set__impl




#undef is_set__impl
;



;
; // 6    PE4 ( OC3B/INT4 ) Digital pin 2 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E5_unused__port_t;

extern E5_unused__port_t const E5_unused__port;

static AKAT_FORCE_INLINE void E5_unused__port__set__impl(u8 state) {
#define set__impl E5_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 5;  //Set PORTE of E5 to 1
    } else {
        PORTE &= ~(1 << 5);  //Set PORTE of E5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E5_unused__port__is_set__impl() {
#define is_set__impl E5_unused__port__is_set__impl
#define set__impl E5_unused__port__set__impl
    return PORTE & (1 << 5);  //Get value of PORTE for E5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E5_unused__port__is_set__impl
#define set__impl E5_unused__port__set__impl

E5_unused__port_t const E5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E5_unused__port__is_set__impl
#define set__impl E5_unused__port__set__impl


;

#define is_set__impl E5_unused__port__is_set__impl
#define set__impl E5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E5_unused__ddr_t;

extern E5_unused__ddr_t const E5_unused__ddr;

static AKAT_FORCE_INLINE void E5_unused__ddr__set__impl(u8 state) {
#define set__impl E5_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 5;  //Set DDRE of E5 to 1
    } else {
        DDRE &= ~(1 << 5);  //Set DDRE of E5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E5_unused__ddr__is_set__impl() {
#define is_set__impl E5_unused__ddr__is_set__impl
#define set__impl E5_unused__ddr__set__impl
    return DDRE & (1 << 5);  //Get value of DDRE for E5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E5_unused__ddr__is_set__impl
#define set__impl E5_unused__ddr__set__impl

E5_unused__ddr_t const E5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E5_unused__ddr__is_set__impl
#define set__impl E5_unused__ddr__set__impl


;

#define is_set__impl E5_unused__ddr__is_set__impl
#define set__impl E5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E5_unused__pin_t;

extern E5_unused__pin_t const E5_unused__pin;

static AKAT_FORCE_INLINE void E5_unused__pin__set__impl(u8 state) {
#define set__impl E5_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 5;  //Set PINE of E5 to 1
    } else {
        PINE &= ~(1 << 5);  //Set PINE of E5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E5_unused__pin__is_set__impl() {
#define is_set__impl E5_unused__pin__is_set__impl
#define set__impl E5_unused__pin__set__impl
    return PINE & (1 << 5);  //Get value of PINE for E5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E5_unused__pin__is_set__impl
#define set__impl E5_unused__pin__set__impl

E5_unused__pin_t const E5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E5_unused__pin__is_set__impl
#define set__impl E5_unused__pin__set__impl


;

#define is_set__impl E5_unused__pin__is_set__impl
#define set__impl E5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E5_unused__init() {
    E5_unused__ddr.set(0);
    E5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E5_unused_t;

extern E5_unused_t const E5_unused;

static AKAT_FORCE_INLINE u8 E5_unused__is_set__impl() {
#define is_set__impl E5_unused__is_set__impl
    return E5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E5_unused__is_set__impl

E5_unused_t const E5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E5_unused__is_set__impl


;

#define is_set__impl E5_unused__is_set__impl




#undef is_set__impl
;



;
; // 7    PE5 ( OC3C/INT5 ) Digital pin 3 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E6_unused__port_t;

extern E6_unused__port_t const E6_unused__port;

static AKAT_FORCE_INLINE void E6_unused__port__set__impl(u8 state) {
#define set__impl E6_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 6;  //Set PORTE of E6 to 1
    } else {
        PORTE &= ~(1 << 6);  //Set PORTE of E6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E6_unused__port__is_set__impl() {
#define is_set__impl E6_unused__port__is_set__impl
#define set__impl E6_unused__port__set__impl
    return PORTE & (1 << 6);  //Get value of PORTE for E6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E6_unused__port__is_set__impl
#define set__impl E6_unused__port__set__impl

E6_unused__port_t const E6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E6_unused__port__is_set__impl
#define set__impl E6_unused__port__set__impl


;

#define is_set__impl E6_unused__port__is_set__impl
#define set__impl E6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E6_unused__ddr_t;

extern E6_unused__ddr_t const E6_unused__ddr;

static AKAT_FORCE_INLINE void E6_unused__ddr__set__impl(u8 state) {
#define set__impl E6_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 6;  //Set DDRE of E6 to 1
    } else {
        DDRE &= ~(1 << 6);  //Set DDRE of E6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E6_unused__ddr__is_set__impl() {
#define is_set__impl E6_unused__ddr__is_set__impl
#define set__impl E6_unused__ddr__set__impl
    return DDRE & (1 << 6);  //Get value of DDRE for E6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E6_unused__ddr__is_set__impl
#define set__impl E6_unused__ddr__set__impl

E6_unused__ddr_t const E6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E6_unused__ddr__is_set__impl
#define set__impl E6_unused__ddr__set__impl


;

#define is_set__impl E6_unused__ddr__is_set__impl
#define set__impl E6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E6_unused__pin_t;

extern E6_unused__pin_t const E6_unused__pin;

static AKAT_FORCE_INLINE void E6_unused__pin__set__impl(u8 state) {
#define set__impl E6_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 6;  //Set PINE of E6 to 1
    } else {
        PINE &= ~(1 << 6);  //Set PINE of E6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E6_unused__pin__is_set__impl() {
#define is_set__impl E6_unused__pin__is_set__impl
#define set__impl E6_unused__pin__set__impl
    return PINE & (1 << 6);  //Get value of PINE for E6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E6_unused__pin__is_set__impl
#define set__impl E6_unused__pin__set__impl

E6_unused__pin_t const E6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E6_unused__pin__is_set__impl
#define set__impl E6_unused__pin__set__impl


;

#define is_set__impl E6_unused__pin__is_set__impl
#define set__impl E6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E6_unused__init() {
    E6_unused__ddr.set(0);
    E6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E6_unused_t;

extern E6_unused_t const E6_unused;

static AKAT_FORCE_INLINE u8 E6_unused__is_set__impl() {
#define is_set__impl E6_unused__is_set__impl
    return E6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E6_unused__is_set__impl

E6_unused_t const E6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E6_unused__is_set__impl


;

#define is_set__impl E6_unused__is_set__impl




#undef is_set__impl
;



;
; // 8    PE6 ( T3/INT6 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E7_unused__port_t;

extern E7_unused__port_t const E7_unused__port;

static AKAT_FORCE_INLINE void E7_unused__port__set__impl(u8 state) {
#define set__impl E7_unused__port__set__impl

    if (state) {
        PORTE |= 1 << 7;  //Set PORTE of E7 to 1
    } else {
        PORTE &= ~(1 << 7);  //Set PORTE of E7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E7_unused__port__is_set__impl() {
#define is_set__impl E7_unused__port__is_set__impl
#define set__impl E7_unused__port__set__impl
    return PORTE & (1 << 7);  //Get value of PORTE for E7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E7_unused__port__is_set__impl
#define set__impl E7_unused__port__set__impl

E7_unused__port_t const E7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl E7_unused__port__is_set__impl
#define set__impl E7_unused__port__set__impl


;

#define is_set__impl E7_unused__port__is_set__impl
#define set__impl E7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E7_unused__ddr_t;

extern E7_unused__ddr_t const E7_unused__ddr;

static AKAT_FORCE_INLINE void E7_unused__ddr__set__impl(u8 state) {
#define set__impl E7_unused__ddr__set__impl

    if (state) {
        DDRE |= 1 << 7;  //Set DDRE of E7 to 1
    } else {
        DDRE &= ~(1 << 7);  //Set DDRE of E7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E7_unused__ddr__is_set__impl() {
#define is_set__impl E7_unused__ddr__is_set__impl
#define set__impl E7_unused__ddr__set__impl
    return DDRE & (1 << 7);  //Get value of DDRE for E7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E7_unused__ddr__is_set__impl
#define set__impl E7_unused__ddr__set__impl

E7_unused__ddr_t const E7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E7_unused__ddr__is_set__impl
#define set__impl E7_unused__ddr__set__impl


;

#define is_set__impl E7_unused__ddr__is_set__impl
#define set__impl E7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} E7_unused__pin_t;

extern E7_unused__pin_t const E7_unused__pin;

static AKAT_FORCE_INLINE void E7_unused__pin__set__impl(u8 state) {
#define set__impl E7_unused__pin__set__impl

    if (state) {
        PINE |= 1 << 7;  //Set PINE of E7 to 1
    } else {
        PINE &= ~(1 << 7);  //Set PINE of E7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 E7_unused__pin__is_set__impl() {
#define is_set__impl E7_unused__pin__is_set__impl
#define set__impl E7_unused__pin__set__impl
    return PINE & (1 << 7);  //Get value of PINE for E7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl E7_unused__pin__is_set__impl
#define set__impl E7_unused__pin__set__impl

E7_unused__pin_t const E7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl E7_unused__pin__is_set__impl
#define set__impl E7_unused__pin__set__impl


;

#define is_set__impl E7_unused__pin__is_set__impl
#define set__impl E7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void E7_unused__init() {
    E7_unused__ddr.set(0);
    E7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} E7_unused_t;

extern E7_unused_t const E7_unused;

static AKAT_FORCE_INLINE u8 E7_unused__is_set__impl() {
#define is_set__impl E7_unused__is_set__impl
    return E7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl E7_unused__is_set__impl

E7_unused_t const E7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl E7_unused__is_set__impl


;

#define is_set__impl E7_unused__is_set__impl




#undef is_set__impl
;



;
; // 9    PE7 ( CLKO/ICP3/INT7 )
// .................. 10   VCC
// .................. 11   GND
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H0_unused__port_t;

extern H0_unused__port_t const H0_unused__port;

static AKAT_FORCE_INLINE void H0_unused__port__set__impl(u8 state) {
#define set__impl H0_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 0;  //Set PORTH of H0 to 1
    } else {
        PORTH &= ~(1 << 0);  //Set PORTH of H0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H0_unused__port__is_set__impl() {
#define is_set__impl H0_unused__port__is_set__impl
#define set__impl H0_unused__port__set__impl
    return PORTH & (1 << 0);  //Get value of PORTH for H0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H0_unused__port__is_set__impl
#define set__impl H0_unused__port__set__impl

H0_unused__port_t const H0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H0_unused__port__is_set__impl
#define set__impl H0_unused__port__set__impl


;

#define is_set__impl H0_unused__port__is_set__impl
#define set__impl H0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H0_unused__ddr_t;

extern H0_unused__ddr_t const H0_unused__ddr;

static AKAT_FORCE_INLINE void H0_unused__ddr__set__impl(u8 state) {
#define set__impl H0_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 0;  //Set DDRH of H0 to 1
    } else {
        DDRH &= ~(1 << 0);  //Set DDRH of H0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H0_unused__ddr__is_set__impl() {
#define is_set__impl H0_unused__ddr__is_set__impl
#define set__impl H0_unused__ddr__set__impl
    return DDRH & (1 << 0);  //Get value of DDRH for H0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H0_unused__ddr__is_set__impl
#define set__impl H0_unused__ddr__set__impl

H0_unused__ddr_t const H0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H0_unused__ddr__is_set__impl
#define set__impl H0_unused__ddr__set__impl


;

#define is_set__impl H0_unused__ddr__is_set__impl
#define set__impl H0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H0_unused__pin_t;

extern H0_unused__pin_t const H0_unused__pin;

static AKAT_FORCE_INLINE void H0_unused__pin__set__impl(u8 state) {
#define set__impl H0_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 0;  //Set PINH of H0 to 1
    } else {
        PINH &= ~(1 << 0);  //Set PINH of H0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H0_unused__pin__is_set__impl() {
#define is_set__impl H0_unused__pin__is_set__impl
#define set__impl H0_unused__pin__set__impl
    return PINH & (1 << 0);  //Get value of PINH for H0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H0_unused__pin__is_set__impl
#define set__impl H0_unused__pin__set__impl

H0_unused__pin_t const H0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H0_unused__pin__is_set__impl
#define set__impl H0_unused__pin__set__impl


;

#define is_set__impl H0_unused__pin__is_set__impl
#define set__impl H0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H0_unused__init() {
    H0_unused__ddr.set(0);
    H0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H0_unused_t;

extern H0_unused_t const H0_unused;

static AKAT_FORCE_INLINE u8 H0_unused__is_set__impl() {
#define is_set__impl H0_unused__is_set__impl
    return H0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H0_unused__is_set__impl

H0_unused_t const H0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H0_unused__is_set__impl


;

#define is_set__impl H0_unused__is_set__impl




#undef is_set__impl
;



;
; // 12   PH0 ( RXD2 ) Digital pin 17 (RX2)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H1_unused__port_t;

extern H1_unused__port_t const H1_unused__port;

static AKAT_FORCE_INLINE void H1_unused__port__set__impl(u8 state) {
#define set__impl H1_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 1;  //Set PORTH of H1 to 1
    } else {
        PORTH &= ~(1 << 1);  //Set PORTH of H1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H1_unused__port__is_set__impl() {
#define is_set__impl H1_unused__port__is_set__impl
#define set__impl H1_unused__port__set__impl
    return PORTH & (1 << 1);  //Get value of PORTH for H1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H1_unused__port__is_set__impl
#define set__impl H1_unused__port__set__impl

H1_unused__port_t const H1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H1_unused__port__is_set__impl
#define set__impl H1_unused__port__set__impl


;

#define is_set__impl H1_unused__port__is_set__impl
#define set__impl H1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H1_unused__ddr_t;

extern H1_unused__ddr_t const H1_unused__ddr;

static AKAT_FORCE_INLINE void H1_unused__ddr__set__impl(u8 state) {
#define set__impl H1_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 1;  //Set DDRH of H1 to 1
    } else {
        DDRH &= ~(1 << 1);  //Set DDRH of H1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H1_unused__ddr__is_set__impl() {
#define is_set__impl H1_unused__ddr__is_set__impl
#define set__impl H1_unused__ddr__set__impl
    return DDRH & (1 << 1);  //Get value of DDRH for H1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H1_unused__ddr__is_set__impl
#define set__impl H1_unused__ddr__set__impl

H1_unused__ddr_t const H1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H1_unused__ddr__is_set__impl
#define set__impl H1_unused__ddr__set__impl


;

#define is_set__impl H1_unused__ddr__is_set__impl
#define set__impl H1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H1_unused__pin_t;

extern H1_unused__pin_t const H1_unused__pin;

static AKAT_FORCE_INLINE void H1_unused__pin__set__impl(u8 state) {
#define set__impl H1_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 1;  //Set PINH of H1 to 1
    } else {
        PINH &= ~(1 << 1);  //Set PINH of H1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H1_unused__pin__is_set__impl() {
#define is_set__impl H1_unused__pin__is_set__impl
#define set__impl H1_unused__pin__set__impl
    return PINH & (1 << 1);  //Get value of PINH for H1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H1_unused__pin__is_set__impl
#define set__impl H1_unused__pin__set__impl

H1_unused__pin_t const H1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H1_unused__pin__is_set__impl
#define set__impl H1_unused__pin__set__impl


;

#define is_set__impl H1_unused__pin__is_set__impl
#define set__impl H1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H1_unused__init() {
    H1_unused__ddr.set(0);
    H1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H1_unused_t;

extern H1_unused_t const H1_unused;

static AKAT_FORCE_INLINE u8 H1_unused__is_set__impl() {
#define is_set__impl H1_unused__is_set__impl
    return H1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H1_unused__is_set__impl

H1_unused_t const H1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H1_unused__is_set__impl


;

#define is_set__impl H1_unused__is_set__impl




#undef is_set__impl
;



;
; // 13   PH1 ( TXD2 ) Digital pin 16 (TX2)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H2_unused__port_t;

extern H2_unused__port_t const H2_unused__port;

static AKAT_FORCE_INLINE void H2_unused__port__set__impl(u8 state) {
#define set__impl H2_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 2;  //Set PORTH of H2 to 1
    } else {
        PORTH &= ~(1 << 2);  //Set PORTH of H2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H2_unused__port__is_set__impl() {
#define is_set__impl H2_unused__port__is_set__impl
#define set__impl H2_unused__port__set__impl
    return PORTH & (1 << 2);  //Get value of PORTH for H2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H2_unused__port__is_set__impl
#define set__impl H2_unused__port__set__impl

H2_unused__port_t const H2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H2_unused__port__is_set__impl
#define set__impl H2_unused__port__set__impl


;

#define is_set__impl H2_unused__port__is_set__impl
#define set__impl H2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H2_unused__ddr_t;

extern H2_unused__ddr_t const H2_unused__ddr;

static AKAT_FORCE_INLINE void H2_unused__ddr__set__impl(u8 state) {
#define set__impl H2_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 2;  //Set DDRH of H2 to 1
    } else {
        DDRH &= ~(1 << 2);  //Set DDRH of H2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H2_unused__ddr__is_set__impl() {
#define is_set__impl H2_unused__ddr__is_set__impl
#define set__impl H2_unused__ddr__set__impl
    return DDRH & (1 << 2);  //Get value of DDRH for H2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H2_unused__ddr__is_set__impl
#define set__impl H2_unused__ddr__set__impl

H2_unused__ddr_t const H2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H2_unused__ddr__is_set__impl
#define set__impl H2_unused__ddr__set__impl


;

#define is_set__impl H2_unused__ddr__is_set__impl
#define set__impl H2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H2_unused__pin_t;

extern H2_unused__pin_t const H2_unused__pin;

static AKAT_FORCE_INLINE void H2_unused__pin__set__impl(u8 state) {
#define set__impl H2_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 2;  //Set PINH of H2 to 1
    } else {
        PINH &= ~(1 << 2);  //Set PINH of H2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H2_unused__pin__is_set__impl() {
#define is_set__impl H2_unused__pin__is_set__impl
#define set__impl H2_unused__pin__set__impl
    return PINH & (1 << 2);  //Get value of PINH for H2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H2_unused__pin__is_set__impl
#define set__impl H2_unused__pin__set__impl

H2_unused__pin_t const H2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H2_unused__pin__is_set__impl
#define set__impl H2_unused__pin__set__impl


;

#define is_set__impl H2_unused__pin__is_set__impl
#define set__impl H2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H2_unused__init() {
    H2_unused__ddr.set(0);
    H2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H2_unused_t;

extern H2_unused_t const H2_unused;

static AKAT_FORCE_INLINE u8 H2_unused__is_set__impl() {
#define is_set__impl H2_unused__is_set__impl
    return H2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H2_unused__is_set__impl

H2_unused_t const H2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H2_unused__is_set__impl


;

#define is_set__impl H2_unused__is_set__impl




#undef is_set__impl
;



;
; // 14   PH2 ( XCK2 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H3_unused__port_t;

extern H3_unused__port_t const H3_unused__port;

static AKAT_FORCE_INLINE void H3_unused__port__set__impl(u8 state) {
#define set__impl H3_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 3;  //Set PORTH of H3 to 1
    } else {
        PORTH &= ~(1 << 3);  //Set PORTH of H3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H3_unused__port__is_set__impl() {
#define is_set__impl H3_unused__port__is_set__impl
#define set__impl H3_unused__port__set__impl
    return PORTH & (1 << 3);  //Get value of PORTH for H3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H3_unused__port__is_set__impl
#define set__impl H3_unused__port__set__impl

H3_unused__port_t const H3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H3_unused__port__is_set__impl
#define set__impl H3_unused__port__set__impl


;

#define is_set__impl H3_unused__port__is_set__impl
#define set__impl H3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H3_unused__ddr_t;

extern H3_unused__ddr_t const H3_unused__ddr;

static AKAT_FORCE_INLINE void H3_unused__ddr__set__impl(u8 state) {
#define set__impl H3_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 3;  //Set DDRH of H3 to 1
    } else {
        DDRH &= ~(1 << 3);  //Set DDRH of H3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H3_unused__ddr__is_set__impl() {
#define is_set__impl H3_unused__ddr__is_set__impl
#define set__impl H3_unused__ddr__set__impl
    return DDRH & (1 << 3);  //Get value of DDRH for H3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H3_unused__ddr__is_set__impl
#define set__impl H3_unused__ddr__set__impl

H3_unused__ddr_t const H3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H3_unused__ddr__is_set__impl
#define set__impl H3_unused__ddr__set__impl


;

#define is_set__impl H3_unused__ddr__is_set__impl
#define set__impl H3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H3_unused__pin_t;

extern H3_unused__pin_t const H3_unused__pin;

static AKAT_FORCE_INLINE void H3_unused__pin__set__impl(u8 state) {
#define set__impl H3_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 3;  //Set PINH of H3 to 1
    } else {
        PINH &= ~(1 << 3);  //Set PINH of H3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H3_unused__pin__is_set__impl() {
#define is_set__impl H3_unused__pin__is_set__impl
#define set__impl H3_unused__pin__set__impl
    return PINH & (1 << 3);  //Get value of PINH for H3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H3_unused__pin__is_set__impl
#define set__impl H3_unused__pin__set__impl

H3_unused__pin_t const H3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H3_unused__pin__is_set__impl
#define set__impl H3_unused__pin__set__impl


;

#define is_set__impl H3_unused__pin__is_set__impl
#define set__impl H3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H3_unused__init() {
    H3_unused__ddr.set(0);
    H3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H3_unused_t;

extern H3_unused_t const H3_unused;

static AKAT_FORCE_INLINE u8 H3_unused__is_set__impl() {
#define is_set__impl H3_unused__is_set__impl
    return H3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H3_unused__is_set__impl

H3_unused_t const H3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H3_unused__is_set__impl


;

#define is_set__impl H3_unused__is_set__impl




#undef is_set__impl
;



;
; // 15   PH3 ( OC4A ) Digital pin 6 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H4_unused__port_t;

extern H4_unused__port_t const H4_unused__port;

static AKAT_FORCE_INLINE void H4_unused__port__set__impl(u8 state) {
#define set__impl H4_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 4;  //Set PORTH of H4 to 1
    } else {
        PORTH &= ~(1 << 4);  //Set PORTH of H4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H4_unused__port__is_set__impl() {
#define is_set__impl H4_unused__port__is_set__impl
#define set__impl H4_unused__port__set__impl
    return PORTH & (1 << 4);  //Get value of PORTH for H4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H4_unused__port__is_set__impl
#define set__impl H4_unused__port__set__impl

H4_unused__port_t const H4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H4_unused__port__is_set__impl
#define set__impl H4_unused__port__set__impl


;

#define is_set__impl H4_unused__port__is_set__impl
#define set__impl H4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H4_unused__ddr_t;

extern H4_unused__ddr_t const H4_unused__ddr;

static AKAT_FORCE_INLINE void H4_unused__ddr__set__impl(u8 state) {
#define set__impl H4_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 4;  //Set DDRH of H4 to 1
    } else {
        DDRH &= ~(1 << 4);  //Set DDRH of H4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H4_unused__ddr__is_set__impl() {
#define is_set__impl H4_unused__ddr__is_set__impl
#define set__impl H4_unused__ddr__set__impl
    return DDRH & (1 << 4);  //Get value of DDRH for H4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H4_unused__ddr__is_set__impl
#define set__impl H4_unused__ddr__set__impl

H4_unused__ddr_t const H4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H4_unused__ddr__is_set__impl
#define set__impl H4_unused__ddr__set__impl


;

#define is_set__impl H4_unused__ddr__is_set__impl
#define set__impl H4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H4_unused__pin_t;

extern H4_unused__pin_t const H4_unused__pin;

static AKAT_FORCE_INLINE void H4_unused__pin__set__impl(u8 state) {
#define set__impl H4_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 4;  //Set PINH of H4 to 1
    } else {
        PINH &= ~(1 << 4);  //Set PINH of H4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H4_unused__pin__is_set__impl() {
#define is_set__impl H4_unused__pin__is_set__impl
#define set__impl H4_unused__pin__set__impl
    return PINH & (1 << 4);  //Get value of PINH for H4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H4_unused__pin__is_set__impl
#define set__impl H4_unused__pin__set__impl

H4_unused__pin_t const H4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H4_unused__pin__is_set__impl
#define set__impl H4_unused__pin__set__impl


;

#define is_set__impl H4_unused__pin__is_set__impl
#define set__impl H4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H4_unused__init() {
    H4_unused__ddr.set(0);
    H4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H4_unused_t;

extern H4_unused_t const H4_unused;

static AKAT_FORCE_INLINE u8 H4_unused__is_set__impl() {
#define is_set__impl H4_unused__is_set__impl
    return H4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H4_unused__is_set__impl

H4_unused_t const H4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H4_unused__is_set__impl


;

#define is_set__impl H4_unused__is_set__impl




#undef is_set__impl
;



;
; // 16   PH4 ( OC4B ) Digital pin 7 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H5_unused__port_t;

extern H5_unused__port_t const H5_unused__port;

static AKAT_FORCE_INLINE void H5_unused__port__set__impl(u8 state) {
#define set__impl H5_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 5;  //Set PORTH of H5 to 1
    } else {
        PORTH &= ~(1 << 5);  //Set PORTH of H5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H5_unused__port__is_set__impl() {
#define is_set__impl H5_unused__port__is_set__impl
#define set__impl H5_unused__port__set__impl
    return PORTH & (1 << 5);  //Get value of PORTH for H5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H5_unused__port__is_set__impl
#define set__impl H5_unused__port__set__impl

H5_unused__port_t const H5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H5_unused__port__is_set__impl
#define set__impl H5_unused__port__set__impl


;

#define is_set__impl H5_unused__port__is_set__impl
#define set__impl H5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H5_unused__ddr_t;

extern H5_unused__ddr_t const H5_unused__ddr;

static AKAT_FORCE_INLINE void H5_unused__ddr__set__impl(u8 state) {
#define set__impl H5_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 5;  //Set DDRH of H5 to 1
    } else {
        DDRH &= ~(1 << 5);  //Set DDRH of H5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H5_unused__ddr__is_set__impl() {
#define is_set__impl H5_unused__ddr__is_set__impl
#define set__impl H5_unused__ddr__set__impl
    return DDRH & (1 << 5);  //Get value of DDRH for H5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H5_unused__ddr__is_set__impl
#define set__impl H5_unused__ddr__set__impl

H5_unused__ddr_t const H5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H5_unused__ddr__is_set__impl
#define set__impl H5_unused__ddr__set__impl


;

#define is_set__impl H5_unused__ddr__is_set__impl
#define set__impl H5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H5_unused__pin_t;

extern H5_unused__pin_t const H5_unused__pin;

static AKAT_FORCE_INLINE void H5_unused__pin__set__impl(u8 state) {
#define set__impl H5_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 5;  //Set PINH of H5 to 1
    } else {
        PINH &= ~(1 << 5);  //Set PINH of H5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H5_unused__pin__is_set__impl() {
#define is_set__impl H5_unused__pin__is_set__impl
#define set__impl H5_unused__pin__set__impl
    return PINH & (1 << 5);  //Get value of PINH for H5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H5_unused__pin__is_set__impl
#define set__impl H5_unused__pin__set__impl

H5_unused__pin_t const H5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H5_unused__pin__is_set__impl
#define set__impl H5_unused__pin__set__impl


;

#define is_set__impl H5_unused__pin__is_set__impl
#define set__impl H5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H5_unused__init() {
    H5_unused__ddr.set(0);
    H5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H5_unused_t;

extern H5_unused_t const H5_unused;

static AKAT_FORCE_INLINE u8 H5_unused__is_set__impl() {
#define is_set__impl H5_unused__is_set__impl
    return H5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H5_unused__is_set__impl

H5_unused_t const H5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H5_unused__is_set__impl


;

#define is_set__impl H5_unused__is_set__impl




#undef is_set__impl
;



;
; // 17   PH5 ( OC4C ) Digital pin 8 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H6_unused__port_t;

extern H6_unused__port_t const H6_unused__port;

static AKAT_FORCE_INLINE void H6_unused__port__set__impl(u8 state) {
#define set__impl H6_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 6;  //Set PORTH of H6 to 1
    } else {
        PORTH &= ~(1 << 6);  //Set PORTH of H6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H6_unused__port__is_set__impl() {
#define is_set__impl H6_unused__port__is_set__impl
#define set__impl H6_unused__port__set__impl
    return PORTH & (1 << 6);  //Get value of PORTH for H6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H6_unused__port__is_set__impl
#define set__impl H6_unused__port__set__impl

H6_unused__port_t const H6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H6_unused__port__is_set__impl
#define set__impl H6_unused__port__set__impl


;

#define is_set__impl H6_unused__port__is_set__impl
#define set__impl H6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H6_unused__ddr_t;

extern H6_unused__ddr_t const H6_unused__ddr;

static AKAT_FORCE_INLINE void H6_unused__ddr__set__impl(u8 state) {
#define set__impl H6_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 6;  //Set DDRH of H6 to 1
    } else {
        DDRH &= ~(1 << 6);  //Set DDRH of H6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H6_unused__ddr__is_set__impl() {
#define is_set__impl H6_unused__ddr__is_set__impl
#define set__impl H6_unused__ddr__set__impl
    return DDRH & (1 << 6);  //Get value of DDRH for H6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H6_unused__ddr__is_set__impl
#define set__impl H6_unused__ddr__set__impl

H6_unused__ddr_t const H6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H6_unused__ddr__is_set__impl
#define set__impl H6_unused__ddr__set__impl


;

#define is_set__impl H6_unused__ddr__is_set__impl
#define set__impl H6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H6_unused__pin_t;

extern H6_unused__pin_t const H6_unused__pin;

static AKAT_FORCE_INLINE void H6_unused__pin__set__impl(u8 state) {
#define set__impl H6_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 6;  //Set PINH of H6 to 1
    } else {
        PINH &= ~(1 << 6);  //Set PINH of H6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H6_unused__pin__is_set__impl() {
#define is_set__impl H6_unused__pin__is_set__impl
#define set__impl H6_unused__pin__set__impl
    return PINH & (1 << 6);  //Get value of PINH for H6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H6_unused__pin__is_set__impl
#define set__impl H6_unused__pin__set__impl

H6_unused__pin_t const H6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H6_unused__pin__is_set__impl
#define set__impl H6_unused__pin__set__impl


;

#define is_set__impl H6_unused__pin__is_set__impl
#define set__impl H6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H6_unused__init() {
    H6_unused__ddr.set(0);
    H6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H6_unused_t;

extern H6_unused_t const H6_unused;

static AKAT_FORCE_INLINE u8 H6_unused__is_set__impl() {
#define is_set__impl H6_unused__is_set__impl
    return H6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H6_unused__is_set__impl

H6_unused_t const H6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H6_unused__is_set__impl


;

#define is_set__impl H6_unused__is_set__impl




#undef is_set__impl
;



;
; // 18   PH6 ( OC2B ) Digital pin 9 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B0_unused__port_t;

extern B0_unused__port_t const B0_unused__port;

static AKAT_FORCE_INLINE void B0_unused__port__set__impl(u8 state) {
#define set__impl B0_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 0;  //Set PORTB of B0 to 1
    } else {
        PORTB &= ~(1 << 0);  //Set PORTB of B0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B0_unused__port__is_set__impl() {
#define is_set__impl B0_unused__port__is_set__impl
#define set__impl B0_unused__port__set__impl
    return PORTB & (1 << 0);  //Get value of PORTB for B0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B0_unused__port__is_set__impl
#define set__impl B0_unused__port__set__impl

B0_unused__port_t const B0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B0_unused__port__is_set__impl
#define set__impl B0_unused__port__set__impl


;

#define is_set__impl B0_unused__port__is_set__impl
#define set__impl B0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B0_unused__ddr_t;

extern B0_unused__ddr_t const B0_unused__ddr;

static AKAT_FORCE_INLINE void B0_unused__ddr__set__impl(u8 state) {
#define set__impl B0_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 0;  //Set DDRB of B0 to 1
    } else {
        DDRB &= ~(1 << 0);  //Set DDRB of B0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B0_unused__ddr__is_set__impl() {
#define is_set__impl B0_unused__ddr__is_set__impl
#define set__impl B0_unused__ddr__set__impl
    return DDRB & (1 << 0);  //Get value of DDRB for B0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B0_unused__ddr__is_set__impl
#define set__impl B0_unused__ddr__set__impl

B0_unused__ddr_t const B0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B0_unused__ddr__is_set__impl
#define set__impl B0_unused__ddr__set__impl


;

#define is_set__impl B0_unused__ddr__is_set__impl
#define set__impl B0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B0_unused__pin_t;

extern B0_unused__pin_t const B0_unused__pin;

static AKAT_FORCE_INLINE void B0_unused__pin__set__impl(u8 state) {
#define set__impl B0_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 0;  //Set PINB of B0 to 1
    } else {
        PINB &= ~(1 << 0);  //Set PINB of B0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B0_unused__pin__is_set__impl() {
#define is_set__impl B0_unused__pin__is_set__impl
#define set__impl B0_unused__pin__set__impl
    return PINB & (1 << 0);  //Get value of PINB for B0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B0_unused__pin__is_set__impl
#define set__impl B0_unused__pin__set__impl

B0_unused__pin_t const B0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B0_unused__pin__is_set__impl
#define set__impl B0_unused__pin__set__impl


;

#define is_set__impl B0_unused__pin__is_set__impl
#define set__impl B0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B0_unused__init() {
    B0_unused__ddr.set(0);
    B0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B0_unused_t;

extern B0_unused_t const B0_unused;

static AKAT_FORCE_INLINE u8 B0_unused__is_set__impl() {
#define is_set__impl B0_unused__is_set__impl
    return B0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B0_unused__is_set__impl

B0_unused_t const B0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B0_unused__is_set__impl


;

#define is_set__impl B0_unused__is_set__impl




#undef is_set__impl
;



;
; // 19   PB0 ( SS/PCINT0 ) Digital pin 53 (SS)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B1_unused__port_t;

extern B1_unused__port_t const B1_unused__port;

static AKAT_FORCE_INLINE void B1_unused__port__set__impl(u8 state) {
#define set__impl B1_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 1;  //Set PORTB of B1 to 1
    } else {
        PORTB &= ~(1 << 1);  //Set PORTB of B1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B1_unused__port__is_set__impl() {
#define is_set__impl B1_unused__port__is_set__impl
#define set__impl B1_unused__port__set__impl
    return PORTB & (1 << 1);  //Get value of PORTB for B1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B1_unused__port__is_set__impl
#define set__impl B1_unused__port__set__impl

B1_unused__port_t const B1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B1_unused__port__is_set__impl
#define set__impl B1_unused__port__set__impl


;

#define is_set__impl B1_unused__port__is_set__impl
#define set__impl B1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B1_unused__ddr_t;

extern B1_unused__ddr_t const B1_unused__ddr;

static AKAT_FORCE_INLINE void B1_unused__ddr__set__impl(u8 state) {
#define set__impl B1_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 1;  //Set DDRB of B1 to 1
    } else {
        DDRB &= ~(1 << 1);  //Set DDRB of B1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B1_unused__ddr__is_set__impl() {
#define is_set__impl B1_unused__ddr__is_set__impl
#define set__impl B1_unused__ddr__set__impl
    return DDRB & (1 << 1);  //Get value of DDRB for B1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B1_unused__ddr__is_set__impl
#define set__impl B1_unused__ddr__set__impl

B1_unused__ddr_t const B1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B1_unused__ddr__is_set__impl
#define set__impl B1_unused__ddr__set__impl


;

#define is_set__impl B1_unused__ddr__is_set__impl
#define set__impl B1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B1_unused__pin_t;

extern B1_unused__pin_t const B1_unused__pin;

static AKAT_FORCE_INLINE void B1_unused__pin__set__impl(u8 state) {
#define set__impl B1_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 1;  //Set PINB of B1 to 1
    } else {
        PINB &= ~(1 << 1);  //Set PINB of B1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B1_unused__pin__is_set__impl() {
#define is_set__impl B1_unused__pin__is_set__impl
#define set__impl B1_unused__pin__set__impl
    return PINB & (1 << 1);  //Get value of PINB for B1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B1_unused__pin__is_set__impl
#define set__impl B1_unused__pin__set__impl

B1_unused__pin_t const B1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B1_unused__pin__is_set__impl
#define set__impl B1_unused__pin__set__impl


;

#define is_set__impl B1_unused__pin__is_set__impl
#define set__impl B1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B1_unused__init() {
    B1_unused__ddr.set(0);
    B1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B1_unused_t;

extern B1_unused_t const B1_unused;

static AKAT_FORCE_INLINE u8 B1_unused__is_set__impl() {
#define is_set__impl B1_unused__is_set__impl
    return B1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B1_unused__is_set__impl

B1_unused_t const B1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B1_unused__is_set__impl


;

#define is_set__impl B1_unused__is_set__impl




#undef is_set__impl
;



;
; // 20   PB1 ( SCK/PCINT1 ) Digital pin 52 (SCK)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B2_unused__port_t;

extern B2_unused__port_t const B2_unused__port;

static AKAT_FORCE_INLINE void B2_unused__port__set__impl(u8 state) {
#define set__impl B2_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 2;  //Set PORTB of B2 to 1
    } else {
        PORTB &= ~(1 << 2);  //Set PORTB of B2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B2_unused__port__is_set__impl() {
#define is_set__impl B2_unused__port__is_set__impl
#define set__impl B2_unused__port__set__impl
    return PORTB & (1 << 2);  //Get value of PORTB for B2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B2_unused__port__is_set__impl
#define set__impl B2_unused__port__set__impl

B2_unused__port_t const B2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B2_unused__port__is_set__impl
#define set__impl B2_unused__port__set__impl


;

#define is_set__impl B2_unused__port__is_set__impl
#define set__impl B2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B2_unused__ddr_t;

extern B2_unused__ddr_t const B2_unused__ddr;

static AKAT_FORCE_INLINE void B2_unused__ddr__set__impl(u8 state) {
#define set__impl B2_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 2;  //Set DDRB of B2 to 1
    } else {
        DDRB &= ~(1 << 2);  //Set DDRB of B2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B2_unused__ddr__is_set__impl() {
#define is_set__impl B2_unused__ddr__is_set__impl
#define set__impl B2_unused__ddr__set__impl
    return DDRB & (1 << 2);  //Get value of DDRB for B2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B2_unused__ddr__is_set__impl
#define set__impl B2_unused__ddr__set__impl

B2_unused__ddr_t const B2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B2_unused__ddr__is_set__impl
#define set__impl B2_unused__ddr__set__impl


;

#define is_set__impl B2_unused__ddr__is_set__impl
#define set__impl B2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B2_unused__pin_t;

extern B2_unused__pin_t const B2_unused__pin;

static AKAT_FORCE_INLINE void B2_unused__pin__set__impl(u8 state) {
#define set__impl B2_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 2;  //Set PINB of B2 to 1
    } else {
        PINB &= ~(1 << 2);  //Set PINB of B2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B2_unused__pin__is_set__impl() {
#define is_set__impl B2_unused__pin__is_set__impl
#define set__impl B2_unused__pin__set__impl
    return PINB & (1 << 2);  //Get value of PINB for B2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B2_unused__pin__is_set__impl
#define set__impl B2_unused__pin__set__impl

B2_unused__pin_t const B2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B2_unused__pin__is_set__impl
#define set__impl B2_unused__pin__set__impl


;

#define is_set__impl B2_unused__pin__is_set__impl
#define set__impl B2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B2_unused__init() {
    B2_unused__ddr.set(0);
    B2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B2_unused_t;

extern B2_unused_t const B2_unused;

static AKAT_FORCE_INLINE u8 B2_unused__is_set__impl() {
#define is_set__impl B2_unused__is_set__impl
    return B2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B2_unused__is_set__impl

B2_unused_t const B2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B2_unused__is_set__impl


;

#define is_set__impl B2_unused__is_set__impl




#undef is_set__impl
;



;
; // 21   PB2 ( MOSI/PCINT2 ) Digital pin 51 (MOSI)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B3_unused__port_t;

extern B3_unused__port_t const B3_unused__port;

static AKAT_FORCE_INLINE void B3_unused__port__set__impl(u8 state) {
#define set__impl B3_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 3;  //Set PORTB of B3 to 1
    } else {
        PORTB &= ~(1 << 3);  //Set PORTB of B3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B3_unused__port__is_set__impl() {
#define is_set__impl B3_unused__port__is_set__impl
#define set__impl B3_unused__port__set__impl
    return PORTB & (1 << 3);  //Get value of PORTB for B3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B3_unused__port__is_set__impl
#define set__impl B3_unused__port__set__impl

B3_unused__port_t const B3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B3_unused__port__is_set__impl
#define set__impl B3_unused__port__set__impl


;

#define is_set__impl B3_unused__port__is_set__impl
#define set__impl B3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B3_unused__ddr_t;

extern B3_unused__ddr_t const B3_unused__ddr;

static AKAT_FORCE_INLINE void B3_unused__ddr__set__impl(u8 state) {
#define set__impl B3_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 3;  //Set DDRB of B3 to 1
    } else {
        DDRB &= ~(1 << 3);  //Set DDRB of B3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B3_unused__ddr__is_set__impl() {
#define is_set__impl B3_unused__ddr__is_set__impl
#define set__impl B3_unused__ddr__set__impl
    return DDRB & (1 << 3);  //Get value of DDRB for B3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B3_unused__ddr__is_set__impl
#define set__impl B3_unused__ddr__set__impl

B3_unused__ddr_t const B3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B3_unused__ddr__is_set__impl
#define set__impl B3_unused__ddr__set__impl


;

#define is_set__impl B3_unused__ddr__is_set__impl
#define set__impl B3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B3_unused__pin_t;

extern B3_unused__pin_t const B3_unused__pin;

static AKAT_FORCE_INLINE void B3_unused__pin__set__impl(u8 state) {
#define set__impl B3_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 3;  //Set PINB of B3 to 1
    } else {
        PINB &= ~(1 << 3);  //Set PINB of B3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B3_unused__pin__is_set__impl() {
#define is_set__impl B3_unused__pin__is_set__impl
#define set__impl B3_unused__pin__set__impl
    return PINB & (1 << 3);  //Get value of PINB for B3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B3_unused__pin__is_set__impl
#define set__impl B3_unused__pin__set__impl

B3_unused__pin_t const B3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B3_unused__pin__is_set__impl
#define set__impl B3_unused__pin__set__impl


;

#define is_set__impl B3_unused__pin__is_set__impl
#define set__impl B3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B3_unused__init() {
    B3_unused__ddr.set(0);
    B3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B3_unused_t;

extern B3_unused_t const B3_unused;

static AKAT_FORCE_INLINE u8 B3_unused__is_set__impl() {
#define is_set__impl B3_unused__is_set__impl
    return B3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B3_unused__is_set__impl

B3_unused_t const B3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B3_unused__is_set__impl


;

#define is_set__impl B3_unused__is_set__impl




#undef is_set__impl
;



;
; // 22   PB3 ( MISO/PCINT3 ) Digital pin 50 (MISO)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B4_unused__port_t;

extern B4_unused__port_t const B4_unused__port;

static AKAT_FORCE_INLINE void B4_unused__port__set__impl(u8 state) {
#define set__impl B4_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 4;  //Set PORTB of B4 to 1
    } else {
        PORTB &= ~(1 << 4);  //Set PORTB of B4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B4_unused__port__is_set__impl() {
#define is_set__impl B4_unused__port__is_set__impl
#define set__impl B4_unused__port__set__impl
    return PORTB & (1 << 4);  //Get value of PORTB for B4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B4_unused__port__is_set__impl
#define set__impl B4_unused__port__set__impl

B4_unused__port_t const B4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B4_unused__port__is_set__impl
#define set__impl B4_unused__port__set__impl


;

#define is_set__impl B4_unused__port__is_set__impl
#define set__impl B4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B4_unused__ddr_t;

extern B4_unused__ddr_t const B4_unused__ddr;

static AKAT_FORCE_INLINE void B4_unused__ddr__set__impl(u8 state) {
#define set__impl B4_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 4;  //Set DDRB of B4 to 1
    } else {
        DDRB &= ~(1 << 4);  //Set DDRB of B4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B4_unused__ddr__is_set__impl() {
#define is_set__impl B4_unused__ddr__is_set__impl
#define set__impl B4_unused__ddr__set__impl
    return DDRB & (1 << 4);  //Get value of DDRB for B4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B4_unused__ddr__is_set__impl
#define set__impl B4_unused__ddr__set__impl

B4_unused__ddr_t const B4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B4_unused__ddr__is_set__impl
#define set__impl B4_unused__ddr__set__impl


;

#define is_set__impl B4_unused__ddr__is_set__impl
#define set__impl B4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B4_unused__pin_t;

extern B4_unused__pin_t const B4_unused__pin;

static AKAT_FORCE_INLINE void B4_unused__pin__set__impl(u8 state) {
#define set__impl B4_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 4;  //Set PINB of B4 to 1
    } else {
        PINB &= ~(1 << 4);  //Set PINB of B4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B4_unused__pin__is_set__impl() {
#define is_set__impl B4_unused__pin__is_set__impl
#define set__impl B4_unused__pin__set__impl
    return PINB & (1 << 4);  //Get value of PINB for B4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B4_unused__pin__is_set__impl
#define set__impl B4_unused__pin__set__impl

B4_unused__pin_t const B4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B4_unused__pin__is_set__impl
#define set__impl B4_unused__pin__set__impl


;

#define is_set__impl B4_unused__pin__is_set__impl
#define set__impl B4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B4_unused__init() {
    B4_unused__ddr.set(0);
    B4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B4_unused_t;

extern B4_unused_t const B4_unused;

static AKAT_FORCE_INLINE u8 B4_unused__is_set__impl() {
#define is_set__impl B4_unused__is_set__impl
    return B4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B4_unused__is_set__impl

B4_unused_t const B4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B4_unused__is_set__impl


;

#define is_set__impl B4_unused__is_set__impl




#undef is_set__impl
;



;
; // 23   PB4 ( OC2A/PCINT4 ) Digital pin 10 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B5_unused__port_t;

extern B5_unused__port_t const B5_unused__port;

static AKAT_FORCE_INLINE void B5_unused__port__set__impl(u8 state) {
#define set__impl B5_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 5;  //Set PORTB of B5 to 1
    } else {
        PORTB &= ~(1 << 5);  //Set PORTB of B5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B5_unused__port__is_set__impl() {
#define is_set__impl B5_unused__port__is_set__impl
#define set__impl B5_unused__port__set__impl
    return PORTB & (1 << 5);  //Get value of PORTB for B5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B5_unused__port__is_set__impl
#define set__impl B5_unused__port__set__impl

B5_unused__port_t const B5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B5_unused__port__is_set__impl
#define set__impl B5_unused__port__set__impl


;

#define is_set__impl B5_unused__port__is_set__impl
#define set__impl B5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B5_unused__ddr_t;

extern B5_unused__ddr_t const B5_unused__ddr;

static AKAT_FORCE_INLINE void B5_unused__ddr__set__impl(u8 state) {
#define set__impl B5_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 5;  //Set DDRB of B5 to 1
    } else {
        DDRB &= ~(1 << 5);  //Set DDRB of B5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B5_unused__ddr__is_set__impl() {
#define is_set__impl B5_unused__ddr__is_set__impl
#define set__impl B5_unused__ddr__set__impl
    return DDRB & (1 << 5);  //Get value of DDRB for B5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B5_unused__ddr__is_set__impl
#define set__impl B5_unused__ddr__set__impl

B5_unused__ddr_t const B5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B5_unused__ddr__is_set__impl
#define set__impl B5_unused__ddr__set__impl


;

#define is_set__impl B5_unused__ddr__is_set__impl
#define set__impl B5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B5_unused__pin_t;

extern B5_unused__pin_t const B5_unused__pin;

static AKAT_FORCE_INLINE void B5_unused__pin__set__impl(u8 state) {
#define set__impl B5_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 5;  //Set PINB of B5 to 1
    } else {
        PINB &= ~(1 << 5);  //Set PINB of B5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B5_unused__pin__is_set__impl() {
#define is_set__impl B5_unused__pin__is_set__impl
#define set__impl B5_unused__pin__set__impl
    return PINB & (1 << 5);  //Get value of PINB for B5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B5_unused__pin__is_set__impl
#define set__impl B5_unused__pin__set__impl

B5_unused__pin_t const B5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B5_unused__pin__is_set__impl
#define set__impl B5_unused__pin__set__impl


;

#define is_set__impl B5_unused__pin__is_set__impl
#define set__impl B5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B5_unused__init() {
    B5_unused__ddr.set(0);
    B5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B5_unused_t;

extern B5_unused_t const B5_unused;

static AKAT_FORCE_INLINE u8 B5_unused__is_set__impl() {
#define is_set__impl B5_unused__is_set__impl
    return B5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B5_unused__is_set__impl

B5_unused_t const B5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B5_unused__is_set__impl


;

#define is_set__impl B5_unused__is_set__impl




#undef is_set__impl
;



;
; // 24   PB5 ( OC1A/PCINT5 ) Digital pin 11 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B6_unused__port_t;

extern B6_unused__port_t const B6_unused__port;

static AKAT_FORCE_INLINE void B6_unused__port__set__impl(u8 state) {
#define set__impl B6_unused__port__set__impl

    if (state) {
        PORTB |= 1 << 6;  //Set PORTB of B6 to 1
    } else {
        PORTB &= ~(1 << 6);  //Set PORTB of B6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B6_unused__port__is_set__impl() {
#define is_set__impl B6_unused__port__is_set__impl
#define set__impl B6_unused__port__set__impl
    return PORTB & (1 << 6);  //Get value of PORTB for B6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B6_unused__port__is_set__impl
#define set__impl B6_unused__port__set__impl

B6_unused__port_t const B6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl B6_unused__port__is_set__impl
#define set__impl B6_unused__port__set__impl


;

#define is_set__impl B6_unused__port__is_set__impl
#define set__impl B6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B6_unused__ddr_t;

extern B6_unused__ddr_t const B6_unused__ddr;

static AKAT_FORCE_INLINE void B6_unused__ddr__set__impl(u8 state) {
#define set__impl B6_unused__ddr__set__impl

    if (state) {
        DDRB |= 1 << 6;  //Set DDRB of B6 to 1
    } else {
        DDRB &= ~(1 << 6);  //Set DDRB of B6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B6_unused__ddr__is_set__impl() {
#define is_set__impl B6_unused__ddr__is_set__impl
#define set__impl B6_unused__ddr__set__impl
    return DDRB & (1 << 6);  //Get value of DDRB for B6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B6_unused__ddr__is_set__impl
#define set__impl B6_unused__ddr__set__impl

B6_unused__ddr_t const B6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B6_unused__ddr__is_set__impl
#define set__impl B6_unused__ddr__set__impl


;

#define is_set__impl B6_unused__ddr__is_set__impl
#define set__impl B6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} B6_unused__pin_t;

extern B6_unused__pin_t const B6_unused__pin;

static AKAT_FORCE_INLINE void B6_unused__pin__set__impl(u8 state) {
#define set__impl B6_unused__pin__set__impl

    if (state) {
        PINB |= 1 << 6;  //Set PINB of B6 to 1
    } else {
        PINB &= ~(1 << 6);  //Set PINB of B6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 B6_unused__pin__is_set__impl() {
#define is_set__impl B6_unused__pin__is_set__impl
#define set__impl B6_unused__pin__set__impl
    return PINB & (1 << 6);  //Get value of PINB for B6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl B6_unused__pin__is_set__impl
#define set__impl B6_unused__pin__set__impl

B6_unused__pin_t const B6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl B6_unused__pin__is_set__impl
#define set__impl B6_unused__pin__set__impl


;

#define is_set__impl B6_unused__pin__is_set__impl
#define set__impl B6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void B6_unused__init() {
    B6_unused__ddr.set(0);
    B6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} B6_unused_t;

extern B6_unused_t const B6_unused;

static AKAT_FORCE_INLINE u8 B6_unused__is_set__impl() {
#define is_set__impl B6_unused__is_set__impl
    return B6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl B6_unused__is_set__impl

B6_unused_t const B6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl B6_unused__is_set__impl


;

#define is_set__impl B6_unused__is_set__impl




#undef is_set__impl
;



;
; // 25   PB6 ( OC1B/PCINT6 ) Digital pin 12 (PWM)
// BLUE LED ......... 26   PB7 ( OC0A/OC1C/PCINT7 ) Digital pin 13 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H7_unused__port_t;

extern H7_unused__port_t const H7_unused__port;

static AKAT_FORCE_INLINE void H7_unused__port__set__impl(u8 state) {
#define set__impl H7_unused__port__set__impl

    if (state) {
        PORTH |= 1 << 7;  //Set PORTH of H7 to 1
    } else {
        PORTH &= ~(1 << 7);  //Set PORTH of H7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H7_unused__port__is_set__impl() {
#define is_set__impl H7_unused__port__is_set__impl
#define set__impl H7_unused__port__set__impl
    return PORTH & (1 << 7);  //Get value of PORTH for H7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H7_unused__port__is_set__impl
#define set__impl H7_unused__port__set__impl

H7_unused__port_t const H7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl H7_unused__port__is_set__impl
#define set__impl H7_unused__port__set__impl


;

#define is_set__impl H7_unused__port__is_set__impl
#define set__impl H7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H7_unused__ddr_t;

extern H7_unused__ddr_t const H7_unused__ddr;

static AKAT_FORCE_INLINE void H7_unused__ddr__set__impl(u8 state) {
#define set__impl H7_unused__ddr__set__impl

    if (state) {
        DDRH |= 1 << 7;  //Set DDRH of H7 to 1
    } else {
        DDRH &= ~(1 << 7);  //Set DDRH of H7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H7_unused__ddr__is_set__impl() {
#define is_set__impl H7_unused__ddr__is_set__impl
#define set__impl H7_unused__ddr__set__impl
    return DDRH & (1 << 7);  //Get value of DDRH for H7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H7_unused__ddr__is_set__impl
#define set__impl H7_unused__ddr__set__impl

H7_unused__ddr_t const H7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H7_unused__ddr__is_set__impl
#define set__impl H7_unused__ddr__set__impl


;

#define is_set__impl H7_unused__ddr__is_set__impl
#define set__impl H7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} H7_unused__pin_t;

extern H7_unused__pin_t const H7_unused__pin;

static AKAT_FORCE_INLINE void H7_unused__pin__set__impl(u8 state) {
#define set__impl H7_unused__pin__set__impl

    if (state) {
        PINH |= 1 << 7;  //Set PINH of H7 to 1
    } else {
        PINH &= ~(1 << 7);  //Set PINH of H7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 H7_unused__pin__is_set__impl() {
#define is_set__impl H7_unused__pin__is_set__impl
#define set__impl H7_unused__pin__set__impl
    return PINH & (1 << 7);  //Get value of PINH for H7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl H7_unused__pin__is_set__impl
#define set__impl H7_unused__pin__set__impl

H7_unused__pin_t const H7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl H7_unused__pin__is_set__impl
#define set__impl H7_unused__pin__set__impl


;

#define is_set__impl H7_unused__pin__is_set__impl
#define set__impl H7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void H7_unused__init() {
    H7_unused__ddr.set(0);
    H7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} H7_unused_t;

extern H7_unused_t const H7_unused;

static AKAT_FORCE_INLINE u8 H7_unused__is_set__impl() {
#define is_set__impl H7_unused__is_set__impl
    return H7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl H7_unused__is_set__impl

H7_unused_t const H7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl H7_unused__is_set__impl


;

#define is_set__impl H7_unused__is_set__impl




#undef is_set__impl
;



;
; // 27   PH7 ( T4 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G3_unused__port_t;

extern G3_unused__port_t const G3_unused__port;

static AKAT_FORCE_INLINE void G3_unused__port__set__impl(u8 state) {
#define set__impl G3_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 3;  //Set PORTG of G3 to 1
    } else {
        PORTG &= ~(1 << 3);  //Set PORTG of G3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G3_unused__port__is_set__impl() {
#define is_set__impl G3_unused__port__is_set__impl
#define set__impl G3_unused__port__set__impl
    return PORTG & (1 << 3);  //Get value of PORTG for G3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G3_unused__port__is_set__impl
#define set__impl G3_unused__port__set__impl

G3_unused__port_t const G3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G3_unused__port__is_set__impl
#define set__impl G3_unused__port__set__impl


;

#define is_set__impl G3_unused__port__is_set__impl
#define set__impl G3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G3_unused__ddr_t;

extern G3_unused__ddr_t const G3_unused__ddr;

static AKAT_FORCE_INLINE void G3_unused__ddr__set__impl(u8 state) {
#define set__impl G3_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 3;  //Set DDRG of G3 to 1
    } else {
        DDRG &= ~(1 << 3);  //Set DDRG of G3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G3_unused__ddr__is_set__impl() {
#define is_set__impl G3_unused__ddr__is_set__impl
#define set__impl G3_unused__ddr__set__impl
    return DDRG & (1 << 3);  //Get value of DDRG for G3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G3_unused__ddr__is_set__impl
#define set__impl G3_unused__ddr__set__impl

G3_unused__ddr_t const G3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G3_unused__ddr__is_set__impl
#define set__impl G3_unused__ddr__set__impl


;

#define is_set__impl G3_unused__ddr__is_set__impl
#define set__impl G3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G3_unused__pin_t;

extern G3_unused__pin_t const G3_unused__pin;

static AKAT_FORCE_INLINE void G3_unused__pin__set__impl(u8 state) {
#define set__impl G3_unused__pin__set__impl

    if (state) {
        PING |= 1 << 3;  //Set PING of G3 to 1
    } else {
        PING &= ~(1 << 3);  //Set PING of G3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G3_unused__pin__is_set__impl() {
#define is_set__impl G3_unused__pin__is_set__impl
#define set__impl G3_unused__pin__set__impl
    return PING & (1 << 3);  //Get value of PING for G3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G3_unused__pin__is_set__impl
#define set__impl G3_unused__pin__set__impl

G3_unused__pin_t const G3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G3_unused__pin__is_set__impl
#define set__impl G3_unused__pin__set__impl


;

#define is_set__impl G3_unused__pin__is_set__impl
#define set__impl G3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G3_unused__init() {
    G3_unused__ddr.set(0);
    G3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G3_unused_t;

extern G3_unused_t const G3_unused;

static AKAT_FORCE_INLINE u8 G3_unused__is_set__impl() {
#define is_set__impl G3_unused__is_set__impl
    return G3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G3_unused__is_set__impl

G3_unused_t const G3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G3_unused__is_set__impl


;

#define is_set__impl G3_unused__is_set__impl




#undef is_set__impl
;



;
; // 28   PG3 ( TOSC2 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G4_unused__port_t;

extern G4_unused__port_t const G4_unused__port;

static AKAT_FORCE_INLINE void G4_unused__port__set__impl(u8 state) {
#define set__impl G4_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 4;  //Set PORTG of G4 to 1
    } else {
        PORTG &= ~(1 << 4);  //Set PORTG of G4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G4_unused__port__is_set__impl() {
#define is_set__impl G4_unused__port__is_set__impl
#define set__impl G4_unused__port__set__impl
    return PORTG & (1 << 4);  //Get value of PORTG for G4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G4_unused__port__is_set__impl
#define set__impl G4_unused__port__set__impl

G4_unused__port_t const G4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G4_unused__port__is_set__impl
#define set__impl G4_unused__port__set__impl


;

#define is_set__impl G4_unused__port__is_set__impl
#define set__impl G4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G4_unused__ddr_t;

extern G4_unused__ddr_t const G4_unused__ddr;

static AKAT_FORCE_INLINE void G4_unused__ddr__set__impl(u8 state) {
#define set__impl G4_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 4;  //Set DDRG of G4 to 1
    } else {
        DDRG &= ~(1 << 4);  //Set DDRG of G4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G4_unused__ddr__is_set__impl() {
#define is_set__impl G4_unused__ddr__is_set__impl
#define set__impl G4_unused__ddr__set__impl
    return DDRG & (1 << 4);  //Get value of DDRG for G4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G4_unused__ddr__is_set__impl
#define set__impl G4_unused__ddr__set__impl

G4_unused__ddr_t const G4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G4_unused__ddr__is_set__impl
#define set__impl G4_unused__ddr__set__impl


;

#define is_set__impl G4_unused__ddr__is_set__impl
#define set__impl G4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G4_unused__pin_t;

extern G4_unused__pin_t const G4_unused__pin;

static AKAT_FORCE_INLINE void G4_unused__pin__set__impl(u8 state) {
#define set__impl G4_unused__pin__set__impl

    if (state) {
        PING |= 1 << 4;  //Set PING of G4 to 1
    } else {
        PING &= ~(1 << 4);  //Set PING of G4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G4_unused__pin__is_set__impl() {
#define is_set__impl G4_unused__pin__is_set__impl
#define set__impl G4_unused__pin__set__impl
    return PING & (1 << 4);  //Get value of PING for G4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G4_unused__pin__is_set__impl
#define set__impl G4_unused__pin__set__impl

G4_unused__pin_t const G4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G4_unused__pin__is_set__impl
#define set__impl G4_unused__pin__set__impl


;

#define is_set__impl G4_unused__pin__is_set__impl
#define set__impl G4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G4_unused__init() {
    G4_unused__ddr.set(0);
    G4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G4_unused_t;

extern G4_unused_t const G4_unused;

static AKAT_FORCE_INLINE u8 G4_unused__is_set__impl() {
#define is_set__impl G4_unused__is_set__impl
    return G4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G4_unused__is_set__impl

G4_unused_t const G4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G4_unused__is_set__impl


;

#define is_set__impl G4_unused__is_set__impl




#undef is_set__impl
;



;
; // 29   PG4 ( TOSC1 )
// .................. 30   RESET
// .................. 31   VCC
// .................. 32   GND
// .................. 33   XTAL2
// .................. 34   XTAL1
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L0_unused__port_t;

extern L0_unused__port_t const L0_unused__port;

static AKAT_FORCE_INLINE void L0_unused__port__set__impl(u8 state) {
#define set__impl L0_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 0;  //Set PORTL of L0 to 1
    } else {
        PORTL &= ~(1 << 0);  //Set PORTL of L0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L0_unused__port__is_set__impl() {
#define is_set__impl L0_unused__port__is_set__impl
#define set__impl L0_unused__port__set__impl
    return PORTL & (1 << 0);  //Get value of PORTL for L0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L0_unused__port__is_set__impl
#define set__impl L0_unused__port__set__impl

L0_unused__port_t const L0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L0_unused__port__is_set__impl
#define set__impl L0_unused__port__set__impl


;

#define is_set__impl L0_unused__port__is_set__impl
#define set__impl L0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L0_unused__ddr_t;

extern L0_unused__ddr_t const L0_unused__ddr;

static AKAT_FORCE_INLINE void L0_unused__ddr__set__impl(u8 state) {
#define set__impl L0_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 0;  //Set DDRL of L0 to 1
    } else {
        DDRL &= ~(1 << 0);  //Set DDRL of L0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L0_unused__ddr__is_set__impl() {
#define is_set__impl L0_unused__ddr__is_set__impl
#define set__impl L0_unused__ddr__set__impl
    return DDRL & (1 << 0);  //Get value of DDRL for L0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L0_unused__ddr__is_set__impl
#define set__impl L0_unused__ddr__set__impl

L0_unused__ddr_t const L0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L0_unused__ddr__is_set__impl
#define set__impl L0_unused__ddr__set__impl


;

#define is_set__impl L0_unused__ddr__is_set__impl
#define set__impl L0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L0_unused__pin_t;

extern L0_unused__pin_t const L0_unused__pin;

static AKAT_FORCE_INLINE void L0_unused__pin__set__impl(u8 state) {
#define set__impl L0_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 0;  //Set PINL of L0 to 1
    } else {
        PINL &= ~(1 << 0);  //Set PINL of L0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L0_unused__pin__is_set__impl() {
#define is_set__impl L0_unused__pin__is_set__impl
#define set__impl L0_unused__pin__set__impl
    return PINL & (1 << 0);  //Get value of PINL for L0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L0_unused__pin__is_set__impl
#define set__impl L0_unused__pin__set__impl

L0_unused__pin_t const L0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L0_unused__pin__is_set__impl
#define set__impl L0_unused__pin__set__impl


;

#define is_set__impl L0_unused__pin__is_set__impl
#define set__impl L0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L0_unused__init() {
    L0_unused__ddr.set(0);
    L0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L0_unused_t;

extern L0_unused_t const L0_unused;

static AKAT_FORCE_INLINE u8 L0_unused__is_set__impl() {
#define is_set__impl L0_unused__is_set__impl
    return L0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L0_unused__is_set__impl

L0_unused_t const L0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L0_unused__is_set__impl


;

#define is_set__impl L0_unused__is_set__impl




#undef is_set__impl
;



;
; // 35   PL0 ( ICP4 ) Digital pin 49
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L1_unused__port_t;

extern L1_unused__port_t const L1_unused__port;

static AKAT_FORCE_INLINE void L1_unused__port__set__impl(u8 state) {
#define set__impl L1_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 1;  //Set PORTL of L1 to 1
    } else {
        PORTL &= ~(1 << 1);  //Set PORTL of L1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L1_unused__port__is_set__impl() {
#define is_set__impl L1_unused__port__is_set__impl
#define set__impl L1_unused__port__set__impl
    return PORTL & (1 << 1);  //Get value of PORTL for L1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L1_unused__port__is_set__impl
#define set__impl L1_unused__port__set__impl

L1_unused__port_t const L1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L1_unused__port__is_set__impl
#define set__impl L1_unused__port__set__impl


;

#define is_set__impl L1_unused__port__is_set__impl
#define set__impl L1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L1_unused__ddr_t;

extern L1_unused__ddr_t const L1_unused__ddr;

static AKAT_FORCE_INLINE void L1_unused__ddr__set__impl(u8 state) {
#define set__impl L1_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 1;  //Set DDRL of L1 to 1
    } else {
        DDRL &= ~(1 << 1);  //Set DDRL of L1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L1_unused__ddr__is_set__impl() {
#define is_set__impl L1_unused__ddr__is_set__impl
#define set__impl L1_unused__ddr__set__impl
    return DDRL & (1 << 1);  //Get value of DDRL for L1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L1_unused__ddr__is_set__impl
#define set__impl L1_unused__ddr__set__impl

L1_unused__ddr_t const L1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L1_unused__ddr__is_set__impl
#define set__impl L1_unused__ddr__set__impl


;

#define is_set__impl L1_unused__ddr__is_set__impl
#define set__impl L1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L1_unused__pin_t;

extern L1_unused__pin_t const L1_unused__pin;

static AKAT_FORCE_INLINE void L1_unused__pin__set__impl(u8 state) {
#define set__impl L1_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 1;  //Set PINL of L1 to 1
    } else {
        PINL &= ~(1 << 1);  //Set PINL of L1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L1_unused__pin__is_set__impl() {
#define is_set__impl L1_unused__pin__is_set__impl
#define set__impl L1_unused__pin__set__impl
    return PINL & (1 << 1);  //Get value of PINL for L1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L1_unused__pin__is_set__impl
#define set__impl L1_unused__pin__set__impl

L1_unused__pin_t const L1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L1_unused__pin__is_set__impl
#define set__impl L1_unused__pin__set__impl


;

#define is_set__impl L1_unused__pin__is_set__impl
#define set__impl L1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L1_unused__init() {
    L1_unused__ddr.set(0);
    L1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L1_unused_t;

extern L1_unused_t const L1_unused;

static AKAT_FORCE_INLINE u8 L1_unused__is_set__impl() {
#define is_set__impl L1_unused__is_set__impl
    return L1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L1_unused__is_set__impl

L1_unused_t const L1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L1_unused__is_set__impl


;

#define is_set__impl L1_unused__is_set__impl




#undef is_set__impl
;



;
; // 36   PL1 ( ICP5 ) Digital pin 48
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L2_unused__port_t;

extern L2_unused__port_t const L2_unused__port;

static AKAT_FORCE_INLINE void L2_unused__port__set__impl(u8 state) {
#define set__impl L2_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 2;  //Set PORTL of L2 to 1
    } else {
        PORTL &= ~(1 << 2);  //Set PORTL of L2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L2_unused__port__is_set__impl() {
#define is_set__impl L2_unused__port__is_set__impl
#define set__impl L2_unused__port__set__impl
    return PORTL & (1 << 2);  //Get value of PORTL for L2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L2_unused__port__is_set__impl
#define set__impl L2_unused__port__set__impl

L2_unused__port_t const L2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L2_unused__port__is_set__impl
#define set__impl L2_unused__port__set__impl


;

#define is_set__impl L2_unused__port__is_set__impl
#define set__impl L2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L2_unused__ddr_t;

extern L2_unused__ddr_t const L2_unused__ddr;

static AKAT_FORCE_INLINE void L2_unused__ddr__set__impl(u8 state) {
#define set__impl L2_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 2;  //Set DDRL of L2 to 1
    } else {
        DDRL &= ~(1 << 2);  //Set DDRL of L2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L2_unused__ddr__is_set__impl() {
#define is_set__impl L2_unused__ddr__is_set__impl
#define set__impl L2_unused__ddr__set__impl
    return DDRL & (1 << 2);  //Get value of DDRL for L2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L2_unused__ddr__is_set__impl
#define set__impl L2_unused__ddr__set__impl

L2_unused__ddr_t const L2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L2_unused__ddr__is_set__impl
#define set__impl L2_unused__ddr__set__impl


;

#define is_set__impl L2_unused__ddr__is_set__impl
#define set__impl L2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L2_unused__pin_t;

extern L2_unused__pin_t const L2_unused__pin;

static AKAT_FORCE_INLINE void L2_unused__pin__set__impl(u8 state) {
#define set__impl L2_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 2;  //Set PINL of L2 to 1
    } else {
        PINL &= ~(1 << 2);  //Set PINL of L2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L2_unused__pin__is_set__impl() {
#define is_set__impl L2_unused__pin__is_set__impl
#define set__impl L2_unused__pin__set__impl
    return PINL & (1 << 2);  //Get value of PINL for L2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L2_unused__pin__is_set__impl
#define set__impl L2_unused__pin__set__impl

L2_unused__pin_t const L2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L2_unused__pin__is_set__impl
#define set__impl L2_unused__pin__set__impl


;

#define is_set__impl L2_unused__pin__is_set__impl
#define set__impl L2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L2_unused__init() {
    L2_unused__ddr.set(0);
    L2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L2_unused_t;

extern L2_unused_t const L2_unused;

static AKAT_FORCE_INLINE u8 L2_unused__is_set__impl() {
#define is_set__impl L2_unused__is_set__impl
    return L2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L2_unused__is_set__impl

L2_unused_t const L2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L2_unused__is_set__impl


;

#define is_set__impl L2_unused__is_set__impl




#undef is_set__impl
;



;
; // 37   PL2 ( T5 ) Digital pin 47
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L3_unused__port_t;

extern L3_unused__port_t const L3_unused__port;

static AKAT_FORCE_INLINE void L3_unused__port__set__impl(u8 state) {
#define set__impl L3_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 3;  //Set PORTL of L3 to 1
    } else {
        PORTL &= ~(1 << 3);  //Set PORTL of L3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L3_unused__port__is_set__impl() {
#define is_set__impl L3_unused__port__is_set__impl
#define set__impl L3_unused__port__set__impl
    return PORTL & (1 << 3);  //Get value of PORTL for L3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L3_unused__port__is_set__impl
#define set__impl L3_unused__port__set__impl

L3_unused__port_t const L3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L3_unused__port__is_set__impl
#define set__impl L3_unused__port__set__impl


;

#define is_set__impl L3_unused__port__is_set__impl
#define set__impl L3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L3_unused__ddr_t;

extern L3_unused__ddr_t const L3_unused__ddr;

static AKAT_FORCE_INLINE void L3_unused__ddr__set__impl(u8 state) {
#define set__impl L3_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 3;  //Set DDRL of L3 to 1
    } else {
        DDRL &= ~(1 << 3);  //Set DDRL of L3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L3_unused__ddr__is_set__impl() {
#define is_set__impl L3_unused__ddr__is_set__impl
#define set__impl L3_unused__ddr__set__impl
    return DDRL & (1 << 3);  //Get value of DDRL for L3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L3_unused__ddr__is_set__impl
#define set__impl L3_unused__ddr__set__impl

L3_unused__ddr_t const L3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L3_unused__ddr__is_set__impl
#define set__impl L3_unused__ddr__set__impl


;

#define is_set__impl L3_unused__ddr__is_set__impl
#define set__impl L3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L3_unused__pin_t;

extern L3_unused__pin_t const L3_unused__pin;

static AKAT_FORCE_INLINE void L3_unused__pin__set__impl(u8 state) {
#define set__impl L3_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 3;  //Set PINL of L3 to 1
    } else {
        PINL &= ~(1 << 3);  //Set PINL of L3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L3_unused__pin__is_set__impl() {
#define is_set__impl L3_unused__pin__is_set__impl
#define set__impl L3_unused__pin__set__impl
    return PINL & (1 << 3);  //Get value of PINL for L3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L3_unused__pin__is_set__impl
#define set__impl L3_unused__pin__set__impl

L3_unused__pin_t const L3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L3_unused__pin__is_set__impl
#define set__impl L3_unused__pin__set__impl


;

#define is_set__impl L3_unused__pin__is_set__impl
#define set__impl L3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L3_unused__init() {
    L3_unused__ddr.set(0);
    L3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L3_unused_t;

extern L3_unused_t const L3_unused;

static AKAT_FORCE_INLINE u8 L3_unused__is_set__impl() {
#define is_set__impl L3_unused__is_set__impl
    return L3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L3_unused__is_set__impl

L3_unused_t const L3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L3_unused__is_set__impl


;

#define is_set__impl L3_unused__is_set__impl




#undef is_set__impl
;



;
; // 38   PL3 ( OC5A ) Digital pin 46 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L4_unused__port_t;

extern L4_unused__port_t const L4_unused__port;

static AKAT_FORCE_INLINE void L4_unused__port__set__impl(u8 state) {
#define set__impl L4_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 4;  //Set PORTL of L4 to 1
    } else {
        PORTL &= ~(1 << 4);  //Set PORTL of L4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L4_unused__port__is_set__impl() {
#define is_set__impl L4_unused__port__is_set__impl
#define set__impl L4_unused__port__set__impl
    return PORTL & (1 << 4);  //Get value of PORTL for L4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L4_unused__port__is_set__impl
#define set__impl L4_unused__port__set__impl

L4_unused__port_t const L4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L4_unused__port__is_set__impl
#define set__impl L4_unused__port__set__impl


;

#define is_set__impl L4_unused__port__is_set__impl
#define set__impl L4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L4_unused__ddr_t;

extern L4_unused__ddr_t const L4_unused__ddr;

static AKAT_FORCE_INLINE void L4_unused__ddr__set__impl(u8 state) {
#define set__impl L4_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 4;  //Set DDRL of L4 to 1
    } else {
        DDRL &= ~(1 << 4);  //Set DDRL of L4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L4_unused__ddr__is_set__impl() {
#define is_set__impl L4_unused__ddr__is_set__impl
#define set__impl L4_unused__ddr__set__impl
    return DDRL & (1 << 4);  //Get value of DDRL for L4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L4_unused__ddr__is_set__impl
#define set__impl L4_unused__ddr__set__impl

L4_unused__ddr_t const L4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L4_unused__ddr__is_set__impl
#define set__impl L4_unused__ddr__set__impl


;

#define is_set__impl L4_unused__ddr__is_set__impl
#define set__impl L4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L4_unused__pin_t;

extern L4_unused__pin_t const L4_unused__pin;

static AKAT_FORCE_INLINE void L4_unused__pin__set__impl(u8 state) {
#define set__impl L4_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 4;  //Set PINL of L4 to 1
    } else {
        PINL &= ~(1 << 4);  //Set PINL of L4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L4_unused__pin__is_set__impl() {
#define is_set__impl L4_unused__pin__is_set__impl
#define set__impl L4_unused__pin__set__impl
    return PINL & (1 << 4);  //Get value of PINL for L4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L4_unused__pin__is_set__impl
#define set__impl L4_unused__pin__set__impl

L4_unused__pin_t const L4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L4_unused__pin__is_set__impl
#define set__impl L4_unused__pin__set__impl


;

#define is_set__impl L4_unused__pin__is_set__impl
#define set__impl L4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L4_unused__init() {
    L4_unused__ddr.set(0);
    L4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L4_unused_t;

extern L4_unused_t const L4_unused;

static AKAT_FORCE_INLINE u8 L4_unused__is_set__impl() {
#define is_set__impl L4_unused__is_set__impl
    return L4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L4_unused__is_set__impl

L4_unused_t const L4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L4_unused__is_set__impl


;

#define is_set__impl L4_unused__is_set__impl




#undef is_set__impl
;



;
; // 39   PL4 ( OC5B ) Digital pin 45 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L5_unused__port_t;

extern L5_unused__port_t const L5_unused__port;

static AKAT_FORCE_INLINE void L5_unused__port__set__impl(u8 state) {
#define set__impl L5_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 5;  //Set PORTL of L5 to 1
    } else {
        PORTL &= ~(1 << 5);  //Set PORTL of L5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L5_unused__port__is_set__impl() {
#define is_set__impl L5_unused__port__is_set__impl
#define set__impl L5_unused__port__set__impl
    return PORTL & (1 << 5);  //Get value of PORTL for L5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L5_unused__port__is_set__impl
#define set__impl L5_unused__port__set__impl

L5_unused__port_t const L5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L5_unused__port__is_set__impl
#define set__impl L5_unused__port__set__impl


;

#define is_set__impl L5_unused__port__is_set__impl
#define set__impl L5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L5_unused__ddr_t;

extern L5_unused__ddr_t const L5_unused__ddr;

static AKAT_FORCE_INLINE void L5_unused__ddr__set__impl(u8 state) {
#define set__impl L5_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 5;  //Set DDRL of L5 to 1
    } else {
        DDRL &= ~(1 << 5);  //Set DDRL of L5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L5_unused__ddr__is_set__impl() {
#define is_set__impl L5_unused__ddr__is_set__impl
#define set__impl L5_unused__ddr__set__impl
    return DDRL & (1 << 5);  //Get value of DDRL for L5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L5_unused__ddr__is_set__impl
#define set__impl L5_unused__ddr__set__impl

L5_unused__ddr_t const L5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L5_unused__ddr__is_set__impl
#define set__impl L5_unused__ddr__set__impl


;

#define is_set__impl L5_unused__ddr__is_set__impl
#define set__impl L5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L5_unused__pin_t;

extern L5_unused__pin_t const L5_unused__pin;

static AKAT_FORCE_INLINE void L5_unused__pin__set__impl(u8 state) {
#define set__impl L5_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 5;  //Set PINL of L5 to 1
    } else {
        PINL &= ~(1 << 5);  //Set PINL of L5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L5_unused__pin__is_set__impl() {
#define is_set__impl L5_unused__pin__is_set__impl
#define set__impl L5_unused__pin__set__impl
    return PINL & (1 << 5);  //Get value of PINL for L5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L5_unused__pin__is_set__impl
#define set__impl L5_unused__pin__set__impl

L5_unused__pin_t const L5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L5_unused__pin__is_set__impl
#define set__impl L5_unused__pin__set__impl


;

#define is_set__impl L5_unused__pin__is_set__impl
#define set__impl L5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L5_unused__init() {
    L5_unused__ddr.set(0);
    L5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L5_unused_t;

extern L5_unused_t const L5_unused;

static AKAT_FORCE_INLINE u8 L5_unused__is_set__impl() {
#define is_set__impl L5_unused__is_set__impl
    return L5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L5_unused__is_set__impl

L5_unused_t const L5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L5_unused__is_set__impl


;

#define is_set__impl L5_unused__is_set__impl




#undef is_set__impl
;



;
; // 40   PL5 ( OC5C ) Digital pin 44 (PWM)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L6_unused__port_t;

extern L6_unused__port_t const L6_unused__port;

static AKAT_FORCE_INLINE void L6_unused__port__set__impl(u8 state) {
#define set__impl L6_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 6;  //Set PORTL of L6 to 1
    } else {
        PORTL &= ~(1 << 6);  //Set PORTL of L6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L6_unused__port__is_set__impl() {
#define is_set__impl L6_unused__port__is_set__impl
#define set__impl L6_unused__port__set__impl
    return PORTL & (1 << 6);  //Get value of PORTL for L6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L6_unused__port__is_set__impl
#define set__impl L6_unused__port__set__impl

L6_unused__port_t const L6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L6_unused__port__is_set__impl
#define set__impl L6_unused__port__set__impl


;

#define is_set__impl L6_unused__port__is_set__impl
#define set__impl L6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L6_unused__ddr_t;

extern L6_unused__ddr_t const L6_unused__ddr;

static AKAT_FORCE_INLINE void L6_unused__ddr__set__impl(u8 state) {
#define set__impl L6_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 6;  //Set DDRL of L6 to 1
    } else {
        DDRL &= ~(1 << 6);  //Set DDRL of L6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L6_unused__ddr__is_set__impl() {
#define is_set__impl L6_unused__ddr__is_set__impl
#define set__impl L6_unused__ddr__set__impl
    return DDRL & (1 << 6);  //Get value of DDRL for L6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L6_unused__ddr__is_set__impl
#define set__impl L6_unused__ddr__set__impl

L6_unused__ddr_t const L6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L6_unused__ddr__is_set__impl
#define set__impl L6_unused__ddr__set__impl


;

#define is_set__impl L6_unused__ddr__is_set__impl
#define set__impl L6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L6_unused__pin_t;

extern L6_unused__pin_t const L6_unused__pin;

static AKAT_FORCE_INLINE void L6_unused__pin__set__impl(u8 state) {
#define set__impl L6_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 6;  //Set PINL of L6 to 1
    } else {
        PINL &= ~(1 << 6);  //Set PINL of L6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L6_unused__pin__is_set__impl() {
#define is_set__impl L6_unused__pin__is_set__impl
#define set__impl L6_unused__pin__set__impl
    return PINL & (1 << 6);  //Get value of PINL for L6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L6_unused__pin__is_set__impl
#define set__impl L6_unused__pin__set__impl

L6_unused__pin_t const L6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L6_unused__pin__is_set__impl
#define set__impl L6_unused__pin__set__impl


;

#define is_set__impl L6_unused__pin__is_set__impl
#define set__impl L6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L6_unused__init() {
    L6_unused__ddr.set(0);
    L6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L6_unused_t;

extern L6_unused_t const L6_unused;

static AKAT_FORCE_INLINE u8 L6_unused__is_set__impl() {
#define is_set__impl L6_unused__is_set__impl
    return L6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L6_unused__is_set__impl

L6_unused_t const L6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L6_unused__is_set__impl


;

#define is_set__impl L6_unused__is_set__impl




#undef is_set__impl
;



;
; // 41   PL6 Digital pin 43
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L7_unused__port_t;

extern L7_unused__port_t const L7_unused__port;

static AKAT_FORCE_INLINE void L7_unused__port__set__impl(u8 state) {
#define set__impl L7_unused__port__set__impl

    if (state) {
        PORTL |= 1 << 7;  //Set PORTL of L7 to 1
    } else {
        PORTL &= ~(1 << 7);  //Set PORTL of L7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L7_unused__port__is_set__impl() {
#define is_set__impl L7_unused__port__is_set__impl
#define set__impl L7_unused__port__set__impl
    return PORTL & (1 << 7);  //Get value of PORTL for L7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L7_unused__port__is_set__impl
#define set__impl L7_unused__port__set__impl

L7_unused__port_t const L7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl L7_unused__port__is_set__impl
#define set__impl L7_unused__port__set__impl


;

#define is_set__impl L7_unused__port__is_set__impl
#define set__impl L7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L7_unused__ddr_t;

extern L7_unused__ddr_t const L7_unused__ddr;

static AKAT_FORCE_INLINE void L7_unused__ddr__set__impl(u8 state) {
#define set__impl L7_unused__ddr__set__impl

    if (state) {
        DDRL |= 1 << 7;  //Set DDRL of L7 to 1
    } else {
        DDRL &= ~(1 << 7);  //Set DDRL of L7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L7_unused__ddr__is_set__impl() {
#define is_set__impl L7_unused__ddr__is_set__impl
#define set__impl L7_unused__ddr__set__impl
    return DDRL & (1 << 7);  //Get value of DDRL for L7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L7_unused__ddr__is_set__impl
#define set__impl L7_unused__ddr__set__impl

L7_unused__ddr_t const L7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L7_unused__ddr__is_set__impl
#define set__impl L7_unused__ddr__set__impl


;

#define is_set__impl L7_unused__ddr__is_set__impl
#define set__impl L7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} L7_unused__pin_t;

extern L7_unused__pin_t const L7_unused__pin;

static AKAT_FORCE_INLINE void L7_unused__pin__set__impl(u8 state) {
#define set__impl L7_unused__pin__set__impl

    if (state) {
        PINL |= 1 << 7;  //Set PINL of L7 to 1
    } else {
        PINL &= ~(1 << 7);  //Set PINL of L7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 L7_unused__pin__is_set__impl() {
#define is_set__impl L7_unused__pin__is_set__impl
#define set__impl L7_unused__pin__set__impl
    return PINL & (1 << 7);  //Get value of PINL for L7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl L7_unused__pin__is_set__impl
#define set__impl L7_unused__pin__set__impl

L7_unused__pin_t const L7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl L7_unused__pin__is_set__impl
#define set__impl L7_unused__pin__set__impl


;

#define is_set__impl L7_unused__pin__is_set__impl
#define set__impl L7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void L7_unused__init() {
    L7_unused__ddr.set(0);
    L7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} L7_unused_t;

extern L7_unused_t const L7_unused;

static AKAT_FORCE_INLINE u8 L7_unused__is_set__impl() {
#define is_set__impl L7_unused__is_set__impl
    return L7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl L7_unused__is_set__impl

L7_unused_t const L7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl L7_unused__is_set__impl


;

#define is_set__impl L7_unused__is_set__impl




#undef is_set__impl
;



;
; // 42   PL7 Digital pin 42
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D0_unused__port_t;

extern D0_unused__port_t const D0_unused__port;

static AKAT_FORCE_INLINE void D0_unused__port__set__impl(u8 state) {
#define set__impl D0_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 0;  //Set PORTD of D0 to 1
    } else {
        PORTD &= ~(1 << 0);  //Set PORTD of D0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D0_unused__port__is_set__impl() {
#define is_set__impl D0_unused__port__is_set__impl
#define set__impl D0_unused__port__set__impl
    return PORTD & (1 << 0);  //Get value of PORTD for D0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D0_unused__port__is_set__impl
#define set__impl D0_unused__port__set__impl

D0_unused__port_t const D0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D0_unused__port__is_set__impl
#define set__impl D0_unused__port__set__impl


;

#define is_set__impl D0_unused__port__is_set__impl
#define set__impl D0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D0_unused__ddr_t;

extern D0_unused__ddr_t const D0_unused__ddr;

static AKAT_FORCE_INLINE void D0_unused__ddr__set__impl(u8 state) {
#define set__impl D0_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 0;  //Set DDRD of D0 to 1
    } else {
        DDRD &= ~(1 << 0);  //Set DDRD of D0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D0_unused__ddr__is_set__impl() {
#define is_set__impl D0_unused__ddr__is_set__impl
#define set__impl D0_unused__ddr__set__impl
    return DDRD & (1 << 0);  //Get value of DDRD for D0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D0_unused__ddr__is_set__impl
#define set__impl D0_unused__ddr__set__impl

D0_unused__ddr_t const D0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D0_unused__ddr__is_set__impl
#define set__impl D0_unused__ddr__set__impl


;

#define is_set__impl D0_unused__ddr__is_set__impl
#define set__impl D0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D0_unused__pin_t;

extern D0_unused__pin_t const D0_unused__pin;

static AKAT_FORCE_INLINE void D0_unused__pin__set__impl(u8 state) {
#define set__impl D0_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 0;  //Set PIND of D0 to 1
    } else {
        PIND &= ~(1 << 0);  //Set PIND of D0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D0_unused__pin__is_set__impl() {
#define is_set__impl D0_unused__pin__is_set__impl
#define set__impl D0_unused__pin__set__impl
    return PIND & (1 << 0);  //Get value of PIND for D0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D0_unused__pin__is_set__impl
#define set__impl D0_unused__pin__set__impl

D0_unused__pin_t const D0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D0_unused__pin__is_set__impl
#define set__impl D0_unused__pin__set__impl


;

#define is_set__impl D0_unused__pin__is_set__impl
#define set__impl D0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D0_unused__init() {
    D0_unused__ddr.set(0);
    D0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D0_unused_t;

extern D0_unused_t const D0_unused;

static AKAT_FORCE_INLINE u8 D0_unused__is_set__impl() {
#define is_set__impl D0_unused__is_set__impl
    return D0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D0_unused__is_set__impl

D0_unused_t const D0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D0_unused__is_set__impl


;

#define is_set__impl D0_unused__is_set__impl




#undef is_set__impl
;



;
; // 43   PD0 ( SCL/INT0 ) Digital pin 21 (SCL)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D1_unused__port_t;

extern D1_unused__port_t const D1_unused__port;

static AKAT_FORCE_INLINE void D1_unused__port__set__impl(u8 state) {
#define set__impl D1_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 1;  //Set PORTD of D1 to 1
    } else {
        PORTD &= ~(1 << 1);  //Set PORTD of D1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D1_unused__port__is_set__impl() {
#define is_set__impl D1_unused__port__is_set__impl
#define set__impl D1_unused__port__set__impl
    return PORTD & (1 << 1);  //Get value of PORTD for D1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D1_unused__port__is_set__impl
#define set__impl D1_unused__port__set__impl

D1_unused__port_t const D1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D1_unused__port__is_set__impl
#define set__impl D1_unused__port__set__impl


;

#define is_set__impl D1_unused__port__is_set__impl
#define set__impl D1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D1_unused__ddr_t;

extern D1_unused__ddr_t const D1_unused__ddr;

static AKAT_FORCE_INLINE void D1_unused__ddr__set__impl(u8 state) {
#define set__impl D1_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 1;  //Set DDRD of D1 to 1
    } else {
        DDRD &= ~(1 << 1);  //Set DDRD of D1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D1_unused__ddr__is_set__impl() {
#define is_set__impl D1_unused__ddr__is_set__impl
#define set__impl D1_unused__ddr__set__impl
    return DDRD & (1 << 1);  //Get value of DDRD for D1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D1_unused__ddr__is_set__impl
#define set__impl D1_unused__ddr__set__impl

D1_unused__ddr_t const D1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D1_unused__ddr__is_set__impl
#define set__impl D1_unused__ddr__set__impl


;

#define is_set__impl D1_unused__ddr__is_set__impl
#define set__impl D1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D1_unused__pin_t;

extern D1_unused__pin_t const D1_unused__pin;

static AKAT_FORCE_INLINE void D1_unused__pin__set__impl(u8 state) {
#define set__impl D1_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 1;  //Set PIND of D1 to 1
    } else {
        PIND &= ~(1 << 1);  //Set PIND of D1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D1_unused__pin__is_set__impl() {
#define is_set__impl D1_unused__pin__is_set__impl
#define set__impl D1_unused__pin__set__impl
    return PIND & (1 << 1);  //Get value of PIND for D1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D1_unused__pin__is_set__impl
#define set__impl D1_unused__pin__set__impl

D1_unused__pin_t const D1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D1_unused__pin__is_set__impl
#define set__impl D1_unused__pin__set__impl


;

#define is_set__impl D1_unused__pin__is_set__impl
#define set__impl D1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D1_unused__init() {
    D1_unused__ddr.set(0);
    D1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D1_unused_t;

extern D1_unused_t const D1_unused;

static AKAT_FORCE_INLINE u8 D1_unused__is_set__impl() {
#define is_set__impl D1_unused__is_set__impl
    return D1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D1_unused__is_set__impl

D1_unused_t const D1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D1_unused__is_set__impl


;

#define is_set__impl D1_unused__is_set__impl




#undef is_set__impl
;



;
; // 44   PD1 ( SDA/INT1 ) Digital pin 20 (SDA)
// USART1 / CO2 ..... 45   PD2 ( RXDI/INT2 ) Digital pin 19 (RX1)
// USART1 / CO2 ..... 46   PD3 ( TXD1/INT3 ) Digital pin 18 (TX1)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D4_unused__port_t;

extern D4_unused__port_t const D4_unused__port;

static AKAT_FORCE_INLINE void D4_unused__port__set__impl(u8 state) {
#define set__impl D4_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 4;  //Set PORTD of D4 to 1
    } else {
        PORTD &= ~(1 << 4);  //Set PORTD of D4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D4_unused__port__is_set__impl() {
#define is_set__impl D4_unused__port__is_set__impl
#define set__impl D4_unused__port__set__impl
    return PORTD & (1 << 4);  //Get value of PORTD for D4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D4_unused__port__is_set__impl
#define set__impl D4_unused__port__set__impl

D4_unused__port_t const D4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D4_unused__port__is_set__impl
#define set__impl D4_unused__port__set__impl


;

#define is_set__impl D4_unused__port__is_set__impl
#define set__impl D4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D4_unused__ddr_t;

extern D4_unused__ddr_t const D4_unused__ddr;

static AKAT_FORCE_INLINE void D4_unused__ddr__set__impl(u8 state) {
#define set__impl D4_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 4;  //Set DDRD of D4 to 1
    } else {
        DDRD &= ~(1 << 4);  //Set DDRD of D4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D4_unused__ddr__is_set__impl() {
#define is_set__impl D4_unused__ddr__is_set__impl
#define set__impl D4_unused__ddr__set__impl
    return DDRD & (1 << 4);  //Get value of DDRD for D4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D4_unused__ddr__is_set__impl
#define set__impl D4_unused__ddr__set__impl

D4_unused__ddr_t const D4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D4_unused__ddr__is_set__impl
#define set__impl D4_unused__ddr__set__impl


;

#define is_set__impl D4_unused__ddr__is_set__impl
#define set__impl D4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D4_unused__pin_t;

extern D4_unused__pin_t const D4_unused__pin;

static AKAT_FORCE_INLINE void D4_unused__pin__set__impl(u8 state) {
#define set__impl D4_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 4;  //Set PIND of D4 to 1
    } else {
        PIND &= ~(1 << 4);  //Set PIND of D4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D4_unused__pin__is_set__impl() {
#define is_set__impl D4_unused__pin__is_set__impl
#define set__impl D4_unused__pin__set__impl
    return PIND & (1 << 4);  //Get value of PIND for D4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D4_unused__pin__is_set__impl
#define set__impl D4_unused__pin__set__impl

D4_unused__pin_t const D4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D4_unused__pin__is_set__impl
#define set__impl D4_unused__pin__set__impl


;

#define is_set__impl D4_unused__pin__is_set__impl
#define set__impl D4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D4_unused__init() {
    D4_unused__ddr.set(0);
    D4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D4_unused_t;

extern D4_unused_t const D4_unused;

static AKAT_FORCE_INLINE u8 D4_unused__is_set__impl() {
#define is_set__impl D4_unused__is_set__impl
    return D4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D4_unused__is_set__impl

D4_unused_t const D4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D4_unused__is_set__impl


;

#define is_set__impl D4_unused__is_set__impl




#undef is_set__impl
;



;
; // 47   PD4 ( ICP1 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D5_unused__port_t;

extern D5_unused__port_t const D5_unused__port;

static AKAT_FORCE_INLINE void D5_unused__port__set__impl(u8 state) {
#define set__impl D5_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 5;  //Set PORTD of D5 to 1
    } else {
        PORTD &= ~(1 << 5);  //Set PORTD of D5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D5_unused__port__is_set__impl() {
#define is_set__impl D5_unused__port__is_set__impl
#define set__impl D5_unused__port__set__impl
    return PORTD & (1 << 5);  //Get value of PORTD for D5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D5_unused__port__is_set__impl
#define set__impl D5_unused__port__set__impl

D5_unused__port_t const D5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D5_unused__port__is_set__impl
#define set__impl D5_unused__port__set__impl


;

#define is_set__impl D5_unused__port__is_set__impl
#define set__impl D5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D5_unused__ddr_t;

extern D5_unused__ddr_t const D5_unused__ddr;

static AKAT_FORCE_INLINE void D5_unused__ddr__set__impl(u8 state) {
#define set__impl D5_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 5;  //Set DDRD of D5 to 1
    } else {
        DDRD &= ~(1 << 5);  //Set DDRD of D5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D5_unused__ddr__is_set__impl() {
#define is_set__impl D5_unused__ddr__is_set__impl
#define set__impl D5_unused__ddr__set__impl
    return DDRD & (1 << 5);  //Get value of DDRD for D5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D5_unused__ddr__is_set__impl
#define set__impl D5_unused__ddr__set__impl

D5_unused__ddr_t const D5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D5_unused__ddr__is_set__impl
#define set__impl D5_unused__ddr__set__impl


;

#define is_set__impl D5_unused__ddr__is_set__impl
#define set__impl D5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D5_unused__pin_t;

extern D5_unused__pin_t const D5_unused__pin;

static AKAT_FORCE_INLINE void D5_unused__pin__set__impl(u8 state) {
#define set__impl D5_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 5;  //Set PIND of D5 to 1
    } else {
        PIND &= ~(1 << 5);  //Set PIND of D5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D5_unused__pin__is_set__impl() {
#define is_set__impl D5_unused__pin__is_set__impl
#define set__impl D5_unused__pin__set__impl
    return PIND & (1 << 5);  //Get value of PIND for D5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D5_unused__pin__is_set__impl
#define set__impl D5_unused__pin__set__impl

D5_unused__pin_t const D5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D5_unused__pin__is_set__impl
#define set__impl D5_unused__pin__set__impl


;

#define is_set__impl D5_unused__pin__is_set__impl
#define set__impl D5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D5_unused__init() {
    D5_unused__ddr.set(0);
    D5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D5_unused_t;

extern D5_unused_t const D5_unused;

static AKAT_FORCE_INLINE u8 D5_unused__is_set__impl() {
#define is_set__impl D5_unused__is_set__impl
    return D5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D5_unused__is_set__impl

D5_unused_t const D5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D5_unused__is_set__impl


;

#define is_set__impl D5_unused__is_set__impl




#undef is_set__impl
;



;
; // 48   PD5 ( XCK1 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D6_unused__port_t;

extern D6_unused__port_t const D6_unused__port;

static AKAT_FORCE_INLINE void D6_unused__port__set__impl(u8 state) {
#define set__impl D6_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 6;  //Set PORTD of D6 to 1
    } else {
        PORTD &= ~(1 << 6);  //Set PORTD of D6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D6_unused__port__is_set__impl() {
#define is_set__impl D6_unused__port__is_set__impl
#define set__impl D6_unused__port__set__impl
    return PORTD & (1 << 6);  //Get value of PORTD for D6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D6_unused__port__is_set__impl
#define set__impl D6_unused__port__set__impl

D6_unused__port_t const D6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D6_unused__port__is_set__impl
#define set__impl D6_unused__port__set__impl


;

#define is_set__impl D6_unused__port__is_set__impl
#define set__impl D6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D6_unused__ddr_t;

extern D6_unused__ddr_t const D6_unused__ddr;

static AKAT_FORCE_INLINE void D6_unused__ddr__set__impl(u8 state) {
#define set__impl D6_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 6;  //Set DDRD of D6 to 1
    } else {
        DDRD &= ~(1 << 6);  //Set DDRD of D6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D6_unused__ddr__is_set__impl() {
#define is_set__impl D6_unused__ddr__is_set__impl
#define set__impl D6_unused__ddr__set__impl
    return DDRD & (1 << 6);  //Get value of DDRD for D6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D6_unused__ddr__is_set__impl
#define set__impl D6_unused__ddr__set__impl

D6_unused__ddr_t const D6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D6_unused__ddr__is_set__impl
#define set__impl D6_unused__ddr__set__impl


;

#define is_set__impl D6_unused__ddr__is_set__impl
#define set__impl D6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D6_unused__pin_t;

extern D6_unused__pin_t const D6_unused__pin;

static AKAT_FORCE_INLINE void D6_unused__pin__set__impl(u8 state) {
#define set__impl D6_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 6;  //Set PIND of D6 to 1
    } else {
        PIND &= ~(1 << 6);  //Set PIND of D6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D6_unused__pin__is_set__impl() {
#define is_set__impl D6_unused__pin__is_set__impl
#define set__impl D6_unused__pin__set__impl
    return PIND & (1 << 6);  //Get value of PIND for D6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D6_unused__pin__is_set__impl
#define set__impl D6_unused__pin__set__impl

D6_unused__pin_t const D6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D6_unused__pin__is_set__impl
#define set__impl D6_unused__pin__set__impl


;

#define is_set__impl D6_unused__pin__is_set__impl
#define set__impl D6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D6_unused__init() {
    D6_unused__ddr.set(0);
    D6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D6_unused_t;

extern D6_unused_t const D6_unused;

static AKAT_FORCE_INLINE u8 D6_unused__is_set__impl() {
#define is_set__impl D6_unused__is_set__impl
    return D6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D6_unused__is_set__impl

D6_unused_t const D6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D6_unused__is_set__impl


;

#define is_set__impl D6_unused__is_set__impl




#undef is_set__impl
;



;
; // 49   PD6 ( T1 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D7_unused__port_t;

extern D7_unused__port_t const D7_unused__port;

static AKAT_FORCE_INLINE void D7_unused__port__set__impl(u8 state) {
#define set__impl D7_unused__port__set__impl

    if (state) {
        PORTD |= 1 << 7;  //Set PORTD of D7 to 1
    } else {
        PORTD &= ~(1 << 7);  //Set PORTD of D7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D7_unused__port__is_set__impl() {
#define is_set__impl D7_unused__port__is_set__impl
#define set__impl D7_unused__port__set__impl
    return PORTD & (1 << 7);  //Get value of PORTD for D7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D7_unused__port__is_set__impl
#define set__impl D7_unused__port__set__impl

D7_unused__port_t const D7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl D7_unused__port__is_set__impl
#define set__impl D7_unused__port__set__impl


;

#define is_set__impl D7_unused__port__is_set__impl
#define set__impl D7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D7_unused__ddr_t;

extern D7_unused__ddr_t const D7_unused__ddr;

static AKAT_FORCE_INLINE void D7_unused__ddr__set__impl(u8 state) {
#define set__impl D7_unused__ddr__set__impl

    if (state) {
        DDRD |= 1 << 7;  //Set DDRD of D7 to 1
    } else {
        DDRD &= ~(1 << 7);  //Set DDRD of D7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D7_unused__ddr__is_set__impl() {
#define is_set__impl D7_unused__ddr__is_set__impl
#define set__impl D7_unused__ddr__set__impl
    return DDRD & (1 << 7);  //Get value of DDRD for D7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D7_unused__ddr__is_set__impl
#define set__impl D7_unused__ddr__set__impl

D7_unused__ddr_t const D7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D7_unused__ddr__is_set__impl
#define set__impl D7_unused__ddr__set__impl


;

#define is_set__impl D7_unused__ddr__is_set__impl
#define set__impl D7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} D7_unused__pin_t;

extern D7_unused__pin_t const D7_unused__pin;

static AKAT_FORCE_INLINE void D7_unused__pin__set__impl(u8 state) {
#define set__impl D7_unused__pin__set__impl

    if (state) {
        PIND |= 1 << 7;  //Set PIND of D7 to 1
    } else {
        PIND &= ~(1 << 7);  //Set PIND of D7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 D7_unused__pin__is_set__impl() {
#define is_set__impl D7_unused__pin__is_set__impl
#define set__impl D7_unused__pin__set__impl
    return PIND & (1 << 7);  //Get value of PIND for D7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl D7_unused__pin__is_set__impl
#define set__impl D7_unused__pin__set__impl

D7_unused__pin_t const D7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl D7_unused__pin__is_set__impl
#define set__impl D7_unused__pin__set__impl


;

#define is_set__impl D7_unused__pin__is_set__impl
#define set__impl D7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void D7_unused__init() {
    D7_unused__ddr.set(0);
    D7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} D7_unused_t;

extern D7_unused_t const D7_unused;

static AKAT_FORCE_INLINE u8 D7_unused__is_set__impl() {
#define is_set__impl D7_unused__is_set__impl
    return D7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl D7_unused__is_set__impl

D7_unused_t const D7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl D7_unused__is_set__impl


;

#define is_set__impl D7_unused__is_set__impl




#undef is_set__impl
;



;
; // 50   PD7 ( T0 ) Digital pin 38
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G0_unused__port_t;

extern G0_unused__port_t const G0_unused__port;

static AKAT_FORCE_INLINE void G0_unused__port__set__impl(u8 state) {
#define set__impl G0_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 0;  //Set PORTG of G0 to 1
    } else {
        PORTG &= ~(1 << 0);  //Set PORTG of G0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G0_unused__port__is_set__impl() {
#define is_set__impl G0_unused__port__is_set__impl
#define set__impl G0_unused__port__set__impl
    return PORTG & (1 << 0);  //Get value of PORTG for G0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G0_unused__port__is_set__impl
#define set__impl G0_unused__port__set__impl

G0_unused__port_t const G0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G0_unused__port__is_set__impl
#define set__impl G0_unused__port__set__impl


;

#define is_set__impl G0_unused__port__is_set__impl
#define set__impl G0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G0_unused__ddr_t;

extern G0_unused__ddr_t const G0_unused__ddr;

static AKAT_FORCE_INLINE void G0_unused__ddr__set__impl(u8 state) {
#define set__impl G0_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 0;  //Set DDRG of G0 to 1
    } else {
        DDRG &= ~(1 << 0);  //Set DDRG of G0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G0_unused__ddr__is_set__impl() {
#define is_set__impl G0_unused__ddr__is_set__impl
#define set__impl G0_unused__ddr__set__impl
    return DDRG & (1 << 0);  //Get value of DDRG for G0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G0_unused__ddr__is_set__impl
#define set__impl G0_unused__ddr__set__impl

G0_unused__ddr_t const G0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G0_unused__ddr__is_set__impl
#define set__impl G0_unused__ddr__set__impl


;

#define is_set__impl G0_unused__ddr__is_set__impl
#define set__impl G0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G0_unused__pin_t;

extern G0_unused__pin_t const G0_unused__pin;

static AKAT_FORCE_INLINE void G0_unused__pin__set__impl(u8 state) {
#define set__impl G0_unused__pin__set__impl

    if (state) {
        PING |= 1 << 0;  //Set PING of G0 to 1
    } else {
        PING &= ~(1 << 0);  //Set PING of G0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G0_unused__pin__is_set__impl() {
#define is_set__impl G0_unused__pin__is_set__impl
#define set__impl G0_unused__pin__set__impl
    return PING & (1 << 0);  //Get value of PING for G0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G0_unused__pin__is_set__impl
#define set__impl G0_unused__pin__set__impl

G0_unused__pin_t const G0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G0_unused__pin__is_set__impl
#define set__impl G0_unused__pin__set__impl


;

#define is_set__impl G0_unused__pin__is_set__impl
#define set__impl G0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G0_unused__init() {
    G0_unused__ddr.set(0);
    G0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G0_unused_t;

extern G0_unused_t const G0_unused;

static AKAT_FORCE_INLINE u8 G0_unused__is_set__impl() {
#define is_set__impl G0_unused__is_set__impl
    return G0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G0_unused__is_set__impl

G0_unused_t const G0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G0_unused__is_set__impl


;

#define is_set__impl G0_unused__is_set__impl




#undef is_set__impl
;



;
; // 51   PG0 ( WR ) Digital pin 41
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G1_unused__port_t;

extern G1_unused__port_t const G1_unused__port;

static AKAT_FORCE_INLINE void G1_unused__port__set__impl(u8 state) {
#define set__impl G1_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 1;  //Set PORTG of G1 to 1
    } else {
        PORTG &= ~(1 << 1);  //Set PORTG of G1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G1_unused__port__is_set__impl() {
#define is_set__impl G1_unused__port__is_set__impl
#define set__impl G1_unused__port__set__impl
    return PORTG & (1 << 1);  //Get value of PORTG for G1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G1_unused__port__is_set__impl
#define set__impl G1_unused__port__set__impl

G1_unused__port_t const G1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G1_unused__port__is_set__impl
#define set__impl G1_unused__port__set__impl


;

#define is_set__impl G1_unused__port__is_set__impl
#define set__impl G1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G1_unused__ddr_t;

extern G1_unused__ddr_t const G1_unused__ddr;

static AKAT_FORCE_INLINE void G1_unused__ddr__set__impl(u8 state) {
#define set__impl G1_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 1;  //Set DDRG of G1 to 1
    } else {
        DDRG &= ~(1 << 1);  //Set DDRG of G1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G1_unused__ddr__is_set__impl() {
#define is_set__impl G1_unused__ddr__is_set__impl
#define set__impl G1_unused__ddr__set__impl
    return DDRG & (1 << 1);  //Get value of DDRG for G1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G1_unused__ddr__is_set__impl
#define set__impl G1_unused__ddr__set__impl

G1_unused__ddr_t const G1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G1_unused__ddr__is_set__impl
#define set__impl G1_unused__ddr__set__impl


;

#define is_set__impl G1_unused__ddr__is_set__impl
#define set__impl G1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G1_unused__pin_t;

extern G1_unused__pin_t const G1_unused__pin;

static AKAT_FORCE_INLINE void G1_unused__pin__set__impl(u8 state) {
#define set__impl G1_unused__pin__set__impl

    if (state) {
        PING |= 1 << 1;  //Set PING of G1 to 1
    } else {
        PING &= ~(1 << 1);  //Set PING of G1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G1_unused__pin__is_set__impl() {
#define is_set__impl G1_unused__pin__is_set__impl
#define set__impl G1_unused__pin__set__impl
    return PING & (1 << 1);  //Get value of PING for G1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G1_unused__pin__is_set__impl
#define set__impl G1_unused__pin__set__impl

G1_unused__pin_t const G1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G1_unused__pin__is_set__impl
#define set__impl G1_unused__pin__set__impl


;

#define is_set__impl G1_unused__pin__is_set__impl
#define set__impl G1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G1_unused__init() {
    G1_unused__ddr.set(0);
    G1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G1_unused_t;

extern G1_unused_t const G1_unused;

static AKAT_FORCE_INLINE u8 G1_unused__is_set__impl() {
#define is_set__impl G1_unused__is_set__impl
    return G1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G1_unused__is_set__impl

G1_unused_t const G1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G1_unused__is_set__impl


;

#define is_set__impl G1_unused__is_set__impl




#undef is_set__impl
;



;
; // 52   PG1 ( RD ) Digital pin 40
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C0_unused__port_t;

extern C0_unused__port_t const C0_unused__port;

static AKAT_FORCE_INLINE void C0_unused__port__set__impl(u8 state) {
#define set__impl C0_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 0;  //Set PORTC of C0 to 1
    } else {
        PORTC &= ~(1 << 0);  //Set PORTC of C0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C0_unused__port__is_set__impl() {
#define is_set__impl C0_unused__port__is_set__impl
#define set__impl C0_unused__port__set__impl
    return PORTC & (1 << 0);  //Get value of PORTC for C0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C0_unused__port__is_set__impl
#define set__impl C0_unused__port__set__impl

C0_unused__port_t const C0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C0_unused__port__is_set__impl
#define set__impl C0_unused__port__set__impl


;

#define is_set__impl C0_unused__port__is_set__impl
#define set__impl C0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C0_unused__ddr_t;

extern C0_unused__ddr_t const C0_unused__ddr;

static AKAT_FORCE_INLINE void C0_unused__ddr__set__impl(u8 state) {
#define set__impl C0_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 0;  //Set DDRC of C0 to 1
    } else {
        DDRC &= ~(1 << 0);  //Set DDRC of C0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C0_unused__ddr__is_set__impl() {
#define is_set__impl C0_unused__ddr__is_set__impl
#define set__impl C0_unused__ddr__set__impl
    return DDRC & (1 << 0);  //Get value of DDRC for C0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C0_unused__ddr__is_set__impl
#define set__impl C0_unused__ddr__set__impl

C0_unused__ddr_t const C0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C0_unused__ddr__is_set__impl
#define set__impl C0_unused__ddr__set__impl


;

#define is_set__impl C0_unused__ddr__is_set__impl
#define set__impl C0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C0_unused__pin_t;

extern C0_unused__pin_t const C0_unused__pin;

static AKAT_FORCE_INLINE void C0_unused__pin__set__impl(u8 state) {
#define set__impl C0_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 0;  //Set PINC of C0 to 1
    } else {
        PINC &= ~(1 << 0);  //Set PINC of C0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C0_unused__pin__is_set__impl() {
#define is_set__impl C0_unused__pin__is_set__impl
#define set__impl C0_unused__pin__set__impl
    return PINC & (1 << 0);  //Get value of PINC for C0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C0_unused__pin__is_set__impl
#define set__impl C0_unused__pin__set__impl

C0_unused__pin_t const C0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C0_unused__pin__is_set__impl
#define set__impl C0_unused__pin__set__impl


;

#define is_set__impl C0_unused__pin__is_set__impl
#define set__impl C0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C0_unused__init() {
    C0_unused__ddr.set(0);
    C0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C0_unused_t;

extern C0_unused_t const C0_unused;

static AKAT_FORCE_INLINE u8 C0_unused__is_set__impl() {
#define is_set__impl C0_unused__is_set__impl
    return C0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C0_unused__is_set__impl

C0_unused_t const C0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C0_unused__is_set__impl


;

#define is_set__impl C0_unused__is_set__impl




#undef is_set__impl
;



;
; // 53   PC0 ( A8 ) Digital pin 37
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C1_unused__port_t;

extern C1_unused__port_t const C1_unused__port;

static AKAT_FORCE_INLINE void C1_unused__port__set__impl(u8 state) {
#define set__impl C1_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 1;  //Set PORTC of C1 to 1
    } else {
        PORTC &= ~(1 << 1);  //Set PORTC of C1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C1_unused__port__is_set__impl() {
#define is_set__impl C1_unused__port__is_set__impl
#define set__impl C1_unused__port__set__impl
    return PORTC & (1 << 1);  //Get value of PORTC for C1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C1_unused__port__is_set__impl
#define set__impl C1_unused__port__set__impl

C1_unused__port_t const C1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C1_unused__port__is_set__impl
#define set__impl C1_unused__port__set__impl


;

#define is_set__impl C1_unused__port__is_set__impl
#define set__impl C1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C1_unused__ddr_t;

extern C1_unused__ddr_t const C1_unused__ddr;

static AKAT_FORCE_INLINE void C1_unused__ddr__set__impl(u8 state) {
#define set__impl C1_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 1;  //Set DDRC of C1 to 1
    } else {
        DDRC &= ~(1 << 1);  //Set DDRC of C1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C1_unused__ddr__is_set__impl() {
#define is_set__impl C1_unused__ddr__is_set__impl
#define set__impl C1_unused__ddr__set__impl
    return DDRC & (1 << 1);  //Get value of DDRC for C1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C1_unused__ddr__is_set__impl
#define set__impl C1_unused__ddr__set__impl

C1_unused__ddr_t const C1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C1_unused__ddr__is_set__impl
#define set__impl C1_unused__ddr__set__impl


;

#define is_set__impl C1_unused__ddr__is_set__impl
#define set__impl C1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C1_unused__pin_t;

extern C1_unused__pin_t const C1_unused__pin;

static AKAT_FORCE_INLINE void C1_unused__pin__set__impl(u8 state) {
#define set__impl C1_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 1;  //Set PINC of C1 to 1
    } else {
        PINC &= ~(1 << 1);  //Set PINC of C1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C1_unused__pin__is_set__impl() {
#define is_set__impl C1_unused__pin__is_set__impl
#define set__impl C1_unused__pin__set__impl
    return PINC & (1 << 1);  //Get value of PINC for C1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C1_unused__pin__is_set__impl
#define set__impl C1_unused__pin__set__impl

C1_unused__pin_t const C1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C1_unused__pin__is_set__impl
#define set__impl C1_unused__pin__set__impl


;

#define is_set__impl C1_unused__pin__is_set__impl
#define set__impl C1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C1_unused__init() {
    C1_unused__ddr.set(0);
    C1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C1_unused_t;

extern C1_unused_t const C1_unused;

static AKAT_FORCE_INLINE u8 C1_unused__is_set__impl() {
#define is_set__impl C1_unused__is_set__impl
    return C1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C1_unused__is_set__impl

C1_unused_t const C1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C1_unused__is_set__impl


;

#define is_set__impl C1_unused__is_set__impl




#undef is_set__impl
;



;
; // 54   PC1 ( A9 ) Digital pin 36
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C2_unused__port_t;

extern C2_unused__port_t const C2_unused__port;

static AKAT_FORCE_INLINE void C2_unused__port__set__impl(u8 state) {
#define set__impl C2_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 2;  //Set PORTC of C2 to 1
    } else {
        PORTC &= ~(1 << 2);  //Set PORTC of C2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C2_unused__port__is_set__impl() {
#define is_set__impl C2_unused__port__is_set__impl
#define set__impl C2_unused__port__set__impl
    return PORTC & (1 << 2);  //Get value of PORTC for C2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C2_unused__port__is_set__impl
#define set__impl C2_unused__port__set__impl

C2_unused__port_t const C2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C2_unused__port__is_set__impl
#define set__impl C2_unused__port__set__impl


;

#define is_set__impl C2_unused__port__is_set__impl
#define set__impl C2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C2_unused__ddr_t;

extern C2_unused__ddr_t const C2_unused__ddr;

static AKAT_FORCE_INLINE void C2_unused__ddr__set__impl(u8 state) {
#define set__impl C2_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 2;  //Set DDRC of C2 to 1
    } else {
        DDRC &= ~(1 << 2);  //Set DDRC of C2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C2_unused__ddr__is_set__impl() {
#define is_set__impl C2_unused__ddr__is_set__impl
#define set__impl C2_unused__ddr__set__impl
    return DDRC & (1 << 2);  //Get value of DDRC for C2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C2_unused__ddr__is_set__impl
#define set__impl C2_unused__ddr__set__impl

C2_unused__ddr_t const C2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C2_unused__ddr__is_set__impl
#define set__impl C2_unused__ddr__set__impl


;

#define is_set__impl C2_unused__ddr__is_set__impl
#define set__impl C2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C2_unused__pin_t;

extern C2_unused__pin_t const C2_unused__pin;

static AKAT_FORCE_INLINE void C2_unused__pin__set__impl(u8 state) {
#define set__impl C2_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 2;  //Set PINC of C2 to 1
    } else {
        PINC &= ~(1 << 2);  //Set PINC of C2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C2_unused__pin__is_set__impl() {
#define is_set__impl C2_unused__pin__is_set__impl
#define set__impl C2_unused__pin__set__impl
    return PINC & (1 << 2);  //Get value of PINC for C2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C2_unused__pin__is_set__impl
#define set__impl C2_unused__pin__set__impl

C2_unused__pin_t const C2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C2_unused__pin__is_set__impl
#define set__impl C2_unused__pin__set__impl


;

#define is_set__impl C2_unused__pin__is_set__impl
#define set__impl C2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C2_unused__init() {
    C2_unused__ddr.set(0);
    C2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C2_unused_t;

extern C2_unused_t const C2_unused;

static AKAT_FORCE_INLINE u8 C2_unused__is_set__impl() {
#define is_set__impl C2_unused__is_set__impl
    return C2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C2_unused__is_set__impl

C2_unused_t const C2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C2_unused__is_set__impl


;

#define is_set__impl C2_unused__is_set__impl




#undef is_set__impl
;



;
; // 55   PC2 ( A10 ) Digital pin 35
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C3_unused__port_t;

extern C3_unused__port_t const C3_unused__port;

static AKAT_FORCE_INLINE void C3_unused__port__set__impl(u8 state) {
#define set__impl C3_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 3;  //Set PORTC of C3 to 1
    } else {
        PORTC &= ~(1 << 3);  //Set PORTC of C3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C3_unused__port__is_set__impl() {
#define is_set__impl C3_unused__port__is_set__impl
#define set__impl C3_unused__port__set__impl
    return PORTC & (1 << 3);  //Get value of PORTC for C3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C3_unused__port__is_set__impl
#define set__impl C3_unused__port__set__impl

C3_unused__port_t const C3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C3_unused__port__is_set__impl
#define set__impl C3_unused__port__set__impl


;

#define is_set__impl C3_unused__port__is_set__impl
#define set__impl C3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C3_unused__ddr_t;

extern C3_unused__ddr_t const C3_unused__ddr;

static AKAT_FORCE_INLINE void C3_unused__ddr__set__impl(u8 state) {
#define set__impl C3_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 3;  //Set DDRC of C3 to 1
    } else {
        DDRC &= ~(1 << 3);  //Set DDRC of C3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C3_unused__ddr__is_set__impl() {
#define is_set__impl C3_unused__ddr__is_set__impl
#define set__impl C3_unused__ddr__set__impl
    return DDRC & (1 << 3);  //Get value of DDRC for C3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C3_unused__ddr__is_set__impl
#define set__impl C3_unused__ddr__set__impl

C3_unused__ddr_t const C3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C3_unused__ddr__is_set__impl
#define set__impl C3_unused__ddr__set__impl


;

#define is_set__impl C3_unused__ddr__is_set__impl
#define set__impl C3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C3_unused__pin_t;

extern C3_unused__pin_t const C3_unused__pin;

static AKAT_FORCE_INLINE void C3_unused__pin__set__impl(u8 state) {
#define set__impl C3_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 3;  //Set PINC of C3 to 1
    } else {
        PINC &= ~(1 << 3);  //Set PINC of C3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C3_unused__pin__is_set__impl() {
#define is_set__impl C3_unused__pin__is_set__impl
#define set__impl C3_unused__pin__set__impl
    return PINC & (1 << 3);  //Get value of PINC for C3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C3_unused__pin__is_set__impl
#define set__impl C3_unused__pin__set__impl

C3_unused__pin_t const C3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C3_unused__pin__is_set__impl
#define set__impl C3_unused__pin__set__impl


;

#define is_set__impl C3_unused__pin__is_set__impl
#define set__impl C3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C3_unused__init() {
    C3_unused__ddr.set(0);
    C3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C3_unused_t;

extern C3_unused_t const C3_unused;

static AKAT_FORCE_INLINE u8 C3_unused__is_set__impl() {
#define is_set__impl C3_unused__is_set__impl
    return C3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C3_unused__is_set__impl

C3_unused_t const C3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C3_unused__is_set__impl


;

#define is_set__impl C3_unused__is_set__impl




#undef is_set__impl
;



;
; // 56   PC3 ( A11 ) Digital pin 34
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C4_unused__port_t;

extern C4_unused__port_t const C4_unused__port;

static AKAT_FORCE_INLINE void C4_unused__port__set__impl(u8 state) {
#define set__impl C4_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 4;  //Set PORTC of C4 to 1
    } else {
        PORTC &= ~(1 << 4);  //Set PORTC of C4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C4_unused__port__is_set__impl() {
#define is_set__impl C4_unused__port__is_set__impl
#define set__impl C4_unused__port__set__impl
    return PORTC & (1 << 4);  //Get value of PORTC for C4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C4_unused__port__is_set__impl
#define set__impl C4_unused__port__set__impl

C4_unused__port_t const C4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C4_unused__port__is_set__impl
#define set__impl C4_unused__port__set__impl


;

#define is_set__impl C4_unused__port__is_set__impl
#define set__impl C4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C4_unused__ddr_t;

extern C4_unused__ddr_t const C4_unused__ddr;

static AKAT_FORCE_INLINE void C4_unused__ddr__set__impl(u8 state) {
#define set__impl C4_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 4;  //Set DDRC of C4 to 1
    } else {
        DDRC &= ~(1 << 4);  //Set DDRC of C4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C4_unused__ddr__is_set__impl() {
#define is_set__impl C4_unused__ddr__is_set__impl
#define set__impl C4_unused__ddr__set__impl
    return DDRC & (1 << 4);  //Get value of DDRC for C4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C4_unused__ddr__is_set__impl
#define set__impl C4_unused__ddr__set__impl

C4_unused__ddr_t const C4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C4_unused__ddr__is_set__impl
#define set__impl C4_unused__ddr__set__impl


;

#define is_set__impl C4_unused__ddr__is_set__impl
#define set__impl C4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C4_unused__pin_t;

extern C4_unused__pin_t const C4_unused__pin;

static AKAT_FORCE_INLINE void C4_unused__pin__set__impl(u8 state) {
#define set__impl C4_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 4;  //Set PINC of C4 to 1
    } else {
        PINC &= ~(1 << 4);  //Set PINC of C4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C4_unused__pin__is_set__impl() {
#define is_set__impl C4_unused__pin__is_set__impl
#define set__impl C4_unused__pin__set__impl
    return PINC & (1 << 4);  //Get value of PINC for C4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C4_unused__pin__is_set__impl
#define set__impl C4_unused__pin__set__impl

C4_unused__pin_t const C4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C4_unused__pin__is_set__impl
#define set__impl C4_unused__pin__set__impl


;

#define is_set__impl C4_unused__pin__is_set__impl
#define set__impl C4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C4_unused__init() {
    C4_unused__ddr.set(0);
    C4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C4_unused_t;

extern C4_unused_t const C4_unused;

static AKAT_FORCE_INLINE u8 C4_unused__is_set__impl() {
#define is_set__impl C4_unused__is_set__impl
    return C4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C4_unused__is_set__impl

C4_unused_t const C4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C4_unused__is_set__impl


;

#define is_set__impl C4_unused__is_set__impl




#undef is_set__impl
;



;
; // 57   PC4 ( A12 ) Digital pin 33
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C5_unused__port_t;

extern C5_unused__port_t const C5_unused__port;

static AKAT_FORCE_INLINE void C5_unused__port__set__impl(u8 state) {
#define set__impl C5_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 5;  //Set PORTC of C5 to 1
    } else {
        PORTC &= ~(1 << 5);  //Set PORTC of C5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C5_unused__port__is_set__impl() {
#define is_set__impl C5_unused__port__is_set__impl
#define set__impl C5_unused__port__set__impl
    return PORTC & (1 << 5);  //Get value of PORTC for C5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C5_unused__port__is_set__impl
#define set__impl C5_unused__port__set__impl

C5_unused__port_t const C5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C5_unused__port__is_set__impl
#define set__impl C5_unused__port__set__impl


;

#define is_set__impl C5_unused__port__is_set__impl
#define set__impl C5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C5_unused__ddr_t;

extern C5_unused__ddr_t const C5_unused__ddr;

static AKAT_FORCE_INLINE void C5_unused__ddr__set__impl(u8 state) {
#define set__impl C5_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 5;  //Set DDRC of C5 to 1
    } else {
        DDRC &= ~(1 << 5);  //Set DDRC of C5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C5_unused__ddr__is_set__impl() {
#define is_set__impl C5_unused__ddr__is_set__impl
#define set__impl C5_unused__ddr__set__impl
    return DDRC & (1 << 5);  //Get value of DDRC for C5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C5_unused__ddr__is_set__impl
#define set__impl C5_unused__ddr__set__impl

C5_unused__ddr_t const C5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C5_unused__ddr__is_set__impl
#define set__impl C5_unused__ddr__set__impl


;

#define is_set__impl C5_unused__ddr__is_set__impl
#define set__impl C5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C5_unused__pin_t;

extern C5_unused__pin_t const C5_unused__pin;

static AKAT_FORCE_INLINE void C5_unused__pin__set__impl(u8 state) {
#define set__impl C5_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 5;  //Set PINC of C5 to 1
    } else {
        PINC &= ~(1 << 5);  //Set PINC of C5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C5_unused__pin__is_set__impl() {
#define is_set__impl C5_unused__pin__is_set__impl
#define set__impl C5_unused__pin__set__impl
    return PINC & (1 << 5);  //Get value of PINC for C5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C5_unused__pin__is_set__impl
#define set__impl C5_unused__pin__set__impl

C5_unused__pin_t const C5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C5_unused__pin__is_set__impl
#define set__impl C5_unused__pin__set__impl


;

#define is_set__impl C5_unused__pin__is_set__impl
#define set__impl C5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C5_unused__init() {
    C5_unused__ddr.set(0);
    C5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C5_unused_t;

extern C5_unused_t const C5_unused;

static AKAT_FORCE_INLINE u8 C5_unused__is_set__impl() {
#define is_set__impl C5_unused__is_set__impl
    return C5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C5_unused__is_set__impl

C5_unused_t const C5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C5_unused__is_set__impl


;

#define is_set__impl C5_unused__is_set__impl




#undef is_set__impl
;



;
; // 58   PC5 ( A13 ) Digital pin 32
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C6_unused__port_t;

extern C6_unused__port_t const C6_unused__port;

static AKAT_FORCE_INLINE void C6_unused__port__set__impl(u8 state) {
#define set__impl C6_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 6;  //Set PORTC of C6 to 1
    } else {
        PORTC &= ~(1 << 6);  //Set PORTC of C6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C6_unused__port__is_set__impl() {
#define is_set__impl C6_unused__port__is_set__impl
#define set__impl C6_unused__port__set__impl
    return PORTC & (1 << 6);  //Get value of PORTC for C6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C6_unused__port__is_set__impl
#define set__impl C6_unused__port__set__impl

C6_unused__port_t const C6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C6_unused__port__is_set__impl
#define set__impl C6_unused__port__set__impl


;

#define is_set__impl C6_unused__port__is_set__impl
#define set__impl C6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C6_unused__ddr_t;

extern C6_unused__ddr_t const C6_unused__ddr;

static AKAT_FORCE_INLINE void C6_unused__ddr__set__impl(u8 state) {
#define set__impl C6_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 6;  //Set DDRC of C6 to 1
    } else {
        DDRC &= ~(1 << 6);  //Set DDRC of C6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C6_unused__ddr__is_set__impl() {
#define is_set__impl C6_unused__ddr__is_set__impl
#define set__impl C6_unused__ddr__set__impl
    return DDRC & (1 << 6);  //Get value of DDRC for C6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C6_unused__ddr__is_set__impl
#define set__impl C6_unused__ddr__set__impl

C6_unused__ddr_t const C6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C6_unused__ddr__is_set__impl
#define set__impl C6_unused__ddr__set__impl


;

#define is_set__impl C6_unused__ddr__is_set__impl
#define set__impl C6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C6_unused__pin_t;

extern C6_unused__pin_t const C6_unused__pin;

static AKAT_FORCE_INLINE void C6_unused__pin__set__impl(u8 state) {
#define set__impl C6_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 6;  //Set PINC of C6 to 1
    } else {
        PINC &= ~(1 << 6);  //Set PINC of C6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C6_unused__pin__is_set__impl() {
#define is_set__impl C6_unused__pin__is_set__impl
#define set__impl C6_unused__pin__set__impl
    return PINC & (1 << 6);  //Get value of PINC for C6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C6_unused__pin__is_set__impl
#define set__impl C6_unused__pin__set__impl

C6_unused__pin_t const C6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C6_unused__pin__is_set__impl
#define set__impl C6_unused__pin__set__impl


;

#define is_set__impl C6_unused__pin__is_set__impl
#define set__impl C6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C6_unused__init() {
    C6_unused__ddr.set(0);
    C6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C6_unused_t;

extern C6_unused_t const C6_unused;

static AKAT_FORCE_INLINE u8 C6_unused__is_set__impl() {
#define is_set__impl C6_unused__is_set__impl
    return C6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C6_unused__is_set__impl

C6_unused_t const C6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C6_unused__is_set__impl


;

#define is_set__impl C6_unused__is_set__impl




#undef is_set__impl
;



;
; // 59   PC6 ( A14 ) Digital pin 31
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C7_unused__port_t;

extern C7_unused__port_t const C7_unused__port;

static AKAT_FORCE_INLINE void C7_unused__port__set__impl(u8 state) {
#define set__impl C7_unused__port__set__impl

    if (state) {
        PORTC |= 1 << 7;  //Set PORTC of C7 to 1
    } else {
        PORTC &= ~(1 << 7);  //Set PORTC of C7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C7_unused__port__is_set__impl() {
#define is_set__impl C7_unused__port__is_set__impl
#define set__impl C7_unused__port__set__impl
    return PORTC & (1 << 7);  //Get value of PORTC for C7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C7_unused__port__is_set__impl
#define set__impl C7_unused__port__set__impl

C7_unused__port_t const C7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl C7_unused__port__is_set__impl
#define set__impl C7_unused__port__set__impl


;

#define is_set__impl C7_unused__port__is_set__impl
#define set__impl C7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C7_unused__ddr_t;

extern C7_unused__ddr_t const C7_unused__ddr;

static AKAT_FORCE_INLINE void C7_unused__ddr__set__impl(u8 state) {
#define set__impl C7_unused__ddr__set__impl

    if (state) {
        DDRC |= 1 << 7;  //Set DDRC of C7 to 1
    } else {
        DDRC &= ~(1 << 7);  //Set DDRC of C7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C7_unused__ddr__is_set__impl() {
#define is_set__impl C7_unused__ddr__is_set__impl
#define set__impl C7_unused__ddr__set__impl
    return DDRC & (1 << 7);  //Get value of DDRC for C7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C7_unused__ddr__is_set__impl
#define set__impl C7_unused__ddr__set__impl

C7_unused__ddr_t const C7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C7_unused__ddr__is_set__impl
#define set__impl C7_unused__ddr__set__impl


;

#define is_set__impl C7_unused__ddr__is_set__impl
#define set__impl C7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} C7_unused__pin_t;

extern C7_unused__pin_t const C7_unused__pin;

static AKAT_FORCE_INLINE void C7_unused__pin__set__impl(u8 state) {
#define set__impl C7_unused__pin__set__impl

    if (state) {
        PINC |= 1 << 7;  //Set PINC of C7 to 1
    } else {
        PINC &= ~(1 << 7);  //Set PINC of C7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 C7_unused__pin__is_set__impl() {
#define is_set__impl C7_unused__pin__is_set__impl
#define set__impl C7_unused__pin__set__impl
    return PINC & (1 << 7);  //Get value of PINC for C7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl C7_unused__pin__is_set__impl
#define set__impl C7_unused__pin__set__impl

C7_unused__pin_t const C7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl C7_unused__pin__is_set__impl
#define set__impl C7_unused__pin__set__impl


;

#define is_set__impl C7_unused__pin__is_set__impl
#define set__impl C7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void C7_unused__init() {
    C7_unused__ddr.set(0);
    C7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} C7_unused_t;

extern C7_unused_t const C7_unused;

static AKAT_FORCE_INLINE u8 C7_unused__is_set__impl() {
#define is_set__impl C7_unused__is_set__impl
    return C7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl C7_unused__is_set__impl

C7_unused_t const C7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl C7_unused__is_set__impl


;

#define is_set__impl C7_unused__is_set__impl




#undef is_set__impl
;



;
; // 60   PC7 ( A15 ) Digital pin 30
// .................. 61   VCC
// .................. 62   GND
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J0_unused__port_t;

extern J0_unused__port_t const J0_unused__port;

static AKAT_FORCE_INLINE void J0_unused__port__set__impl(u8 state) {
#define set__impl J0_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 0;  //Set PORTJ of J0 to 1
    } else {
        PORTJ &= ~(1 << 0);  //Set PORTJ of J0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J0_unused__port__is_set__impl() {
#define is_set__impl J0_unused__port__is_set__impl
#define set__impl J0_unused__port__set__impl
    return PORTJ & (1 << 0);  //Get value of PORTJ for J0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J0_unused__port__is_set__impl
#define set__impl J0_unused__port__set__impl

J0_unused__port_t const J0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J0_unused__port__is_set__impl
#define set__impl J0_unused__port__set__impl


;

#define is_set__impl J0_unused__port__is_set__impl
#define set__impl J0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J0_unused__ddr_t;

extern J0_unused__ddr_t const J0_unused__ddr;

static AKAT_FORCE_INLINE void J0_unused__ddr__set__impl(u8 state) {
#define set__impl J0_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 0;  //Set DDRJ of J0 to 1
    } else {
        DDRJ &= ~(1 << 0);  //Set DDRJ of J0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J0_unused__ddr__is_set__impl() {
#define is_set__impl J0_unused__ddr__is_set__impl
#define set__impl J0_unused__ddr__set__impl
    return DDRJ & (1 << 0);  //Get value of DDRJ for J0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J0_unused__ddr__is_set__impl
#define set__impl J0_unused__ddr__set__impl

J0_unused__ddr_t const J0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J0_unused__ddr__is_set__impl
#define set__impl J0_unused__ddr__set__impl


;

#define is_set__impl J0_unused__ddr__is_set__impl
#define set__impl J0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J0_unused__pin_t;

extern J0_unused__pin_t const J0_unused__pin;

static AKAT_FORCE_INLINE void J0_unused__pin__set__impl(u8 state) {
#define set__impl J0_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 0;  //Set PINJ of J0 to 1
    } else {
        PINJ &= ~(1 << 0);  //Set PINJ of J0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J0_unused__pin__is_set__impl() {
#define is_set__impl J0_unused__pin__is_set__impl
#define set__impl J0_unused__pin__set__impl
    return PINJ & (1 << 0);  //Get value of PINJ for J0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J0_unused__pin__is_set__impl
#define set__impl J0_unused__pin__set__impl

J0_unused__pin_t const J0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J0_unused__pin__is_set__impl
#define set__impl J0_unused__pin__set__impl


;

#define is_set__impl J0_unused__pin__is_set__impl
#define set__impl J0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J0_unused__init() {
    J0_unused__ddr.set(0);
    J0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J0_unused_t;

extern J0_unused_t const J0_unused;

static AKAT_FORCE_INLINE u8 J0_unused__is_set__impl() {
#define is_set__impl J0_unused__is_set__impl
    return J0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J0_unused__is_set__impl

J0_unused_t const J0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J0_unused__is_set__impl


;

#define is_set__impl J0_unused__is_set__impl




#undef is_set__impl
;



;
; // 63   PJ0 ( RXD3/PCINT9 ) Digital pin 15 (RX3)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J1_unused__port_t;

extern J1_unused__port_t const J1_unused__port;

static AKAT_FORCE_INLINE void J1_unused__port__set__impl(u8 state) {
#define set__impl J1_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 1;  //Set PORTJ of J1 to 1
    } else {
        PORTJ &= ~(1 << 1);  //Set PORTJ of J1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J1_unused__port__is_set__impl() {
#define is_set__impl J1_unused__port__is_set__impl
#define set__impl J1_unused__port__set__impl
    return PORTJ & (1 << 1);  //Get value of PORTJ for J1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J1_unused__port__is_set__impl
#define set__impl J1_unused__port__set__impl

J1_unused__port_t const J1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J1_unused__port__is_set__impl
#define set__impl J1_unused__port__set__impl


;

#define is_set__impl J1_unused__port__is_set__impl
#define set__impl J1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J1_unused__ddr_t;

extern J1_unused__ddr_t const J1_unused__ddr;

static AKAT_FORCE_INLINE void J1_unused__ddr__set__impl(u8 state) {
#define set__impl J1_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 1;  //Set DDRJ of J1 to 1
    } else {
        DDRJ &= ~(1 << 1);  //Set DDRJ of J1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J1_unused__ddr__is_set__impl() {
#define is_set__impl J1_unused__ddr__is_set__impl
#define set__impl J1_unused__ddr__set__impl
    return DDRJ & (1 << 1);  //Get value of DDRJ for J1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J1_unused__ddr__is_set__impl
#define set__impl J1_unused__ddr__set__impl

J1_unused__ddr_t const J1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J1_unused__ddr__is_set__impl
#define set__impl J1_unused__ddr__set__impl


;

#define is_set__impl J1_unused__ddr__is_set__impl
#define set__impl J1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J1_unused__pin_t;

extern J1_unused__pin_t const J1_unused__pin;

static AKAT_FORCE_INLINE void J1_unused__pin__set__impl(u8 state) {
#define set__impl J1_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 1;  //Set PINJ of J1 to 1
    } else {
        PINJ &= ~(1 << 1);  //Set PINJ of J1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J1_unused__pin__is_set__impl() {
#define is_set__impl J1_unused__pin__is_set__impl
#define set__impl J1_unused__pin__set__impl
    return PINJ & (1 << 1);  //Get value of PINJ for J1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J1_unused__pin__is_set__impl
#define set__impl J1_unused__pin__set__impl

J1_unused__pin_t const J1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J1_unused__pin__is_set__impl
#define set__impl J1_unused__pin__set__impl


;

#define is_set__impl J1_unused__pin__is_set__impl
#define set__impl J1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J1_unused__init() {
    J1_unused__ddr.set(0);
    J1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J1_unused_t;

extern J1_unused_t const J1_unused;

static AKAT_FORCE_INLINE u8 J1_unused__is_set__impl() {
#define is_set__impl J1_unused__is_set__impl
    return J1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J1_unused__is_set__impl

J1_unused_t const J1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J1_unused__is_set__impl


;

#define is_set__impl J1_unused__is_set__impl




#undef is_set__impl
;



;
; // 64   PJ1 ( TXD3/PCINT10 ) Digital pin 14 (TX3)
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J2_unused__port_t;

extern J2_unused__port_t const J2_unused__port;

static AKAT_FORCE_INLINE void J2_unused__port__set__impl(u8 state) {
#define set__impl J2_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 2;  //Set PORTJ of J2 to 1
    } else {
        PORTJ &= ~(1 << 2);  //Set PORTJ of J2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J2_unused__port__is_set__impl() {
#define is_set__impl J2_unused__port__is_set__impl
#define set__impl J2_unused__port__set__impl
    return PORTJ & (1 << 2);  //Get value of PORTJ for J2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J2_unused__port__is_set__impl
#define set__impl J2_unused__port__set__impl

J2_unused__port_t const J2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J2_unused__port__is_set__impl
#define set__impl J2_unused__port__set__impl


;

#define is_set__impl J2_unused__port__is_set__impl
#define set__impl J2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J2_unused__ddr_t;

extern J2_unused__ddr_t const J2_unused__ddr;

static AKAT_FORCE_INLINE void J2_unused__ddr__set__impl(u8 state) {
#define set__impl J2_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 2;  //Set DDRJ of J2 to 1
    } else {
        DDRJ &= ~(1 << 2);  //Set DDRJ of J2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J2_unused__ddr__is_set__impl() {
#define is_set__impl J2_unused__ddr__is_set__impl
#define set__impl J2_unused__ddr__set__impl
    return DDRJ & (1 << 2);  //Get value of DDRJ for J2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J2_unused__ddr__is_set__impl
#define set__impl J2_unused__ddr__set__impl

J2_unused__ddr_t const J2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J2_unused__ddr__is_set__impl
#define set__impl J2_unused__ddr__set__impl


;

#define is_set__impl J2_unused__ddr__is_set__impl
#define set__impl J2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J2_unused__pin_t;

extern J2_unused__pin_t const J2_unused__pin;

static AKAT_FORCE_INLINE void J2_unused__pin__set__impl(u8 state) {
#define set__impl J2_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 2;  //Set PINJ of J2 to 1
    } else {
        PINJ &= ~(1 << 2);  //Set PINJ of J2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J2_unused__pin__is_set__impl() {
#define is_set__impl J2_unused__pin__is_set__impl
#define set__impl J2_unused__pin__set__impl
    return PINJ & (1 << 2);  //Get value of PINJ for J2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J2_unused__pin__is_set__impl
#define set__impl J2_unused__pin__set__impl

J2_unused__pin_t const J2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J2_unused__pin__is_set__impl
#define set__impl J2_unused__pin__set__impl


;

#define is_set__impl J2_unused__pin__is_set__impl
#define set__impl J2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J2_unused__init() {
    J2_unused__ddr.set(0);
    J2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J2_unused_t;

extern J2_unused_t const J2_unused;

static AKAT_FORCE_INLINE u8 J2_unused__is_set__impl() {
#define is_set__impl J2_unused__is_set__impl
    return J2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J2_unused__is_set__impl

J2_unused_t const J2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J2_unused__is_set__impl


;

#define is_set__impl J2_unused__is_set__impl




#undef is_set__impl
;



;
; // 65   PJ2 ( XCK3/PCINT11 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J3_unused__port_t;

extern J3_unused__port_t const J3_unused__port;

static AKAT_FORCE_INLINE void J3_unused__port__set__impl(u8 state) {
#define set__impl J3_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 3;  //Set PORTJ of J3 to 1
    } else {
        PORTJ &= ~(1 << 3);  //Set PORTJ of J3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J3_unused__port__is_set__impl() {
#define is_set__impl J3_unused__port__is_set__impl
#define set__impl J3_unused__port__set__impl
    return PORTJ & (1 << 3);  //Get value of PORTJ for J3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J3_unused__port__is_set__impl
#define set__impl J3_unused__port__set__impl

J3_unused__port_t const J3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J3_unused__port__is_set__impl
#define set__impl J3_unused__port__set__impl


;

#define is_set__impl J3_unused__port__is_set__impl
#define set__impl J3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J3_unused__ddr_t;

extern J3_unused__ddr_t const J3_unused__ddr;

static AKAT_FORCE_INLINE void J3_unused__ddr__set__impl(u8 state) {
#define set__impl J3_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 3;  //Set DDRJ of J3 to 1
    } else {
        DDRJ &= ~(1 << 3);  //Set DDRJ of J3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J3_unused__ddr__is_set__impl() {
#define is_set__impl J3_unused__ddr__is_set__impl
#define set__impl J3_unused__ddr__set__impl
    return DDRJ & (1 << 3);  //Get value of DDRJ for J3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J3_unused__ddr__is_set__impl
#define set__impl J3_unused__ddr__set__impl

J3_unused__ddr_t const J3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J3_unused__ddr__is_set__impl
#define set__impl J3_unused__ddr__set__impl


;

#define is_set__impl J3_unused__ddr__is_set__impl
#define set__impl J3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J3_unused__pin_t;

extern J3_unused__pin_t const J3_unused__pin;

static AKAT_FORCE_INLINE void J3_unused__pin__set__impl(u8 state) {
#define set__impl J3_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 3;  //Set PINJ of J3 to 1
    } else {
        PINJ &= ~(1 << 3);  //Set PINJ of J3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J3_unused__pin__is_set__impl() {
#define is_set__impl J3_unused__pin__is_set__impl
#define set__impl J3_unused__pin__set__impl
    return PINJ & (1 << 3);  //Get value of PINJ for J3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J3_unused__pin__is_set__impl
#define set__impl J3_unused__pin__set__impl

J3_unused__pin_t const J3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J3_unused__pin__is_set__impl
#define set__impl J3_unused__pin__set__impl


;

#define is_set__impl J3_unused__pin__is_set__impl
#define set__impl J3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J3_unused__init() {
    J3_unused__ddr.set(0);
    J3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J3_unused_t;

extern J3_unused_t const J3_unused;

static AKAT_FORCE_INLINE u8 J3_unused__is_set__impl() {
#define is_set__impl J3_unused__is_set__impl
    return J3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J3_unused__is_set__impl

J3_unused_t const J3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J3_unused__is_set__impl


;

#define is_set__impl J3_unused__is_set__impl




#undef is_set__impl
;



;
; // 66   PJ3 ( PCINT12 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J4_unused__port_t;

extern J4_unused__port_t const J4_unused__port;

static AKAT_FORCE_INLINE void J4_unused__port__set__impl(u8 state) {
#define set__impl J4_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 4;  //Set PORTJ of J4 to 1
    } else {
        PORTJ &= ~(1 << 4);  //Set PORTJ of J4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J4_unused__port__is_set__impl() {
#define is_set__impl J4_unused__port__is_set__impl
#define set__impl J4_unused__port__set__impl
    return PORTJ & (1 << 4);  //Get value of PORTJ for J4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J4_unused__port__is_set__impl
#define set__impl J4_unused__port__set__impl

J4_unused__port_t const J4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J4_unused__port__is_set__impl
#define set__impl J4_unused__port__set__impl


;

#define is_set__impl J4_unused__port__is_set__impl
#define set__impl J4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J4_unused__ddr_t;

extern J4_unused__ddr_t const J4_unused__ddr;

static AKAT_FORCE_INLINE void J4_unused__ddr__set__impl(u8 state) {
#define set__impl J4_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 4;  //Set DDRJ of J4 to 1
    } else {
        DDRJ &= ~(1 << 4);  //Set DDRJ of J4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J4_unused__ddr__is_set__impl() {
#define is_set__impl J4_unused__ddr__is_set__impl
#define set__impl J4_unused__ddr__set__impl
    return DDRJ & (1 << 4);  //Get value of DDRJ for J4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J4_unused__ddr__is_set__impl
#define set__impl J4_unused__ddr__set__impl

J4_unused__ddr_t const J4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J4_unused__ddr__is_set__impl
#define set__impl J4_unused__ddr__set__impl


;

#define is_set__impl J4_unused__ddr__is_set__impl
#define set__impl J4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J4_unused__pin_t;

extern J4_unused__pin_t const J4_unused__pin;

static AKAT_FORCE_INLINE void J4_unused__pin__set__impl(u8 state) {
#define set__impl J4_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 4;  //Set PINJ of J4 to 1
    } else {
        PINJ &= ~(1 << 4);  //Set PINJ of J4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J4_unused__pin__is_set__impl() {
#define is_set__impl J4_unused__pin__is_set__impl
#define set__impl J4_unused__pin__set__impl
    return PINJ & (1 << 4);  //Get value of PINJ for J4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J4_unused__pin__is_set__impl
#define set__impl J4_unused__pin__set__impl

J4_unused__pin_t const J4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J4_unused__pin__is_set__impl
#define set__impl J4_unused__pin__set__impl


;

#define is_set__impl J4_unused__pin__is_set__impl
#define set__impl J4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J4_unused__init() {
    J4_unused__ddr.set(0);
    J4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J4_unused_t;

extern J4_unused_t const J4_unused;

static AKAT_FORCE_INLINE u8 J4_unused__is_set__impl() {
#define is_set__impl J4_unused__is_set__impl
    return J4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J4_unused__is_set__impl

J4_unused_t const J4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J4_unused__is_set__impl


;

#define is_set__impl J4_unused__is_set__impl




#undef is_set__impl
;



;
; // 67   PJ4 ( PCINT13 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J5_unused__port_t;

extern J5_unused__port_t const J5_unused__port;

static AKAT_FORCE_INLINE void J5_unused__port__set__impl(u8 state) {
#define set__impl J5_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 5;  //Set PORTJ of J5 to 1
    } else {
        PORTJ &= ~(1 << 5);  //Set PORTJ of J5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J5_unused__port__is_set__impl() {
#define is_set__impl J5_unused__port__is_set__impl
#define set__impl J5_unused__port__set__impl
    return PORTJ & (1 << 5);  //Get value of PORTJ for J5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J5_unused__port__is_set__impl
#define set__impl J5_unused__port__set__impl

J5_unused__port_t const J5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J5_unused__port__is_set__impl
#define set__impl J5_unused__port__set__impl


;

#define is_set__impl J5_unused__port__is_set__impl
#define set__impl J5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J5_unused__ddr_t;

extern J5_unused__ddr_t const J5_unused__ddr;

static AKAT_FORCE_INLINE void J5_unused__ddr__set__impl(u8 state) {
#define set__impl J5_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 5;  //Set DDRJ of J5 to 1
    } else {
        DDRJ &= ~(1 << 5);  //Set DDRJ of J5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J5_unused__ddr__is_set__impl() {
#define is_set__impl J5_unused__ddr__is_set__impl
#define set__impl J5_unused__ddr__set__impl
    return DDRJ & (1 << 5);  //Get value of DDRJ for J5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J5_unused__ddr__is_set__impl
#define set__impl J5_unused__ddr__set__impl

J5_unused__ddr_t const J5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J5_unused__ddr__is_set__impl
#define set__impl J5_unused__ddr__set__impl


;

#define is_set__impl J5_unused__ddr__is_set__impl
#define set__impl J5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J5_unused__pin_t;

extern J5_unused__pin_t const J5_unused__pin;

static AKAT_FORCE_INLINE void J5_unused__pin__set__impl(u8 state) {
#define set__impl J5_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 5;  //Set PINJ of J5 to 1
    } else {
        PINJ &= ~(1 << 5);  //Set PINJ of J5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J5_unused__pin__is_set__impl() {
#define is_set__impl J5_unused__pin__is_set__impl
#define set__impl J5_unused__pin__set__impl
    return PINJ & (1 << 5);  //Get value of PINJ for J5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J5_unused__pin__is_set__impl
#define set__impl J5_unused__pin__set__impl

J5_unused__pin_t const J5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J5_unused__pin__is_set__impl
#define set__impl J5_unused__pin__set__impl


;

#define is_set__impl J5_unused__pin__is_set__impl
#define set__impl J5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J5_unused__init() {
    J5_unused__ddr.set(0);
    J5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J5_unused_t;

extern J5_unused_t const J5_unused;

static AKAT_FORCE_INLINE u8 J5_unused__is_set__impl() {
#define is_set__impl J5_unused__is_set__impl
    return J5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J5_unused__is_set__impl

J5_unused_t const J5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J5_unused__is_set__impl


;

#define is_set__impl J5_unused__is_set__impl




#undef is_set__impl
;



;
; // 68   PJ5 ( PCINT14 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J6_unused__port_t;

extern J6_unused__port_t const J6_unused__port;

static AKAT_FORCE_INLINE void J6_unused__port__set__impl(u8 state) {
#define set__impl J6_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 6;  //Set PORTJ of J6 to 1
    } else {
        PORTJ &= ~(1 << 6);  //Set PORTJ of J6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J6_unused__port__is_set__impl() {
#define is_set__impl J6_unused__port__is_set__impl
#define set__impl J6_unused__port__set__impl
    return PORTJ & (1 << 6);  //Get value of PORTJ for J6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J6_unused__port__is_set__impl
#define set__impl J6_unused__port__set__impl

J6_unused__port_t const J6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J6_unused__port__is_set__impl
#define set__impl J6_unused__port__set__impl


;

#define is_set__impl J6_unused__port__is_set__impl
#define set__impl J6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J6_unused__ddr_t;

extern J6_unused__ddr_t const J6_unused__ddr;

static AKAT_FORCE_INLINE void J6_unused__ddr__set__impl(u8 state) {
#define set__impl J6_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 6;  //Set DDRJ of J6 to 1
    } else {
        DDRJ &= ~(1 << 6);  //Set DDRJ of J6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J6_unused__ddr__is_set__impl() {
#define is_set__impl J6_unused__ddr__is_set__impl
#define set__impl J6_unused__ddr__set__impl
    return DDRJ & (1 << 6);  //Get value of DDRJ for J6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J6_unused__ddr__is_set__impl
#define set__impl J6_unused__ddr__set__impl

J6_unused__ddr_t const J6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J6_unused__ddr__is_set__impl
#define set__impl J6_unused__ddr__set__impl


;

#define is_set__impl J6_unused__ddr__is_set__impl
#define set__impl J6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J6_unused__pin_t;

extern J6_unused__pin_t const J6_unused__pin;

static AKAT_FORCE_INLINE void J6_unused__pin__set__impl(u8 state) {
#define set__impl J6_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 6;  //Set PINJ of J6 to 1
    } else {
        PINJ &= ~(1 << 6);  //Set PINJ of J6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J6_unused__pin__is_set__impl() {
#define is_set__impl J6_unused__pin__is_set__impl
#define set__impl J6_unused__pin__set__impl
    return PINJ & (1 << 6);  //Get value of PINJ for J6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J6_unused__pin__is_set__impl
#define set__impl J6_unused__pin__set__impl

J6_unused__pin_t const J6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J6_unused__pin__is_set__impl
#define set__impl J6_unused__pin__set__impl


;

#define is_set__impl J6_unused__pin__is_set__impl
#define set__impl J6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J6_unused__init() {
    J6_unused__ddr.set(0);
    J6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J6_unused_t;

extern J6_unused_t const J6_unused;

static AKAT_FORCE_INLINE u8 J6_unused__is_set__impl() {
#define is_set__impl J6_unused__is_set__impl
    return J6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J6_unused__is_set__impl

J6_unused_t const J6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J6_unused__is_set__impl


;

#define is_set__impl J6_unused__is_set__impl




#undef is_set__impl
;



;
; // 69   PJ6 ( PCINT 15 )
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G2_unused__port_t;

extern G2_unused__port_t const G2_unused__port;

static AKAT_FORCE_INLINE void G2_unused__port__set__impl(u8 state) {
#define set__impl G2_unused__port__set__impl

    if (state) {
        PORTG |= 1 << 2;  //Set PORTG of G2 to 1
    } else {
        PORTG &= ~(1 << 2);  //Set PORTG of G2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G2_unused__port__is_set__impl() {
#define is_set__impl G2_unused__port__is_set__impl
#define set__impl G2_unused__port__set__impl
    return PORTG & (1 << 2);  //Get value of PORTG for G2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G2_unused__port__is_set__impl
#define set__impl G2_unused__port__set__impl

G2_unused__port_t const G2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl G2_unused__port__is_set__impl
#define set__impl G2_unused__port__set__impl


;

#define is_set__impl G2_unused__port__is_set__impl
#define set__impl G2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G2_unused__ddr_t;

extern G2_unused__ddr_t const G2_unused__ddr;

static AKAT_FORCE_INLINE void G2_unused__ddr__set__impl(u8 state) {
#define set__impl G2_unused__ddr__set__impl

    if (state) {
        DDRG |= 1 << 2;  //Set DDRG of G2 to 1
    } else {
        DDRG &= ~(1 << 2);  //Set DDRG of G2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G2_unused__ddr__is_set__impl() {
#define is_set__impl G2_unused__ddr__is_set__impl
#define set__impl G2_unused__ddr__set__impl
    return DDRG & (1 << 2);  //Get value of DDRG for G2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G2_unused__ddr__is_set__impl
#define set__impl G2_unused__ddr__set__impl

G2_unused__ddr_t const G2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G2_unused__ddr__is_set__impl
#define set__impl G2_unused__ddr__set__impl


;

#define is_set__impl G2_unused__ddr__is_set__impl
#define set__impl G2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} G2_unused__pin_t;

extern G2_unused__pin_t const G2_unused__pin;

static AKAT_FORCE_INLINE void G2_unused__pin__set__impl(u8 state) {
#define set__impl G2_unused__pin__set__impl

    if (state) {
        PING |= 1 << 2;  //Set PING of G2 to 1
    } else {
        PING &= ~(1 << 2);  //Set PING of G2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 G2_unused__pin__is_set__impl() {
#define is_set__impl G2_unused__pin__is_set__impl
#define set__impl G2_unused__pin__set__impl
    return PING & (1 << 2);  //Get value of PING for G2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl G2_unused__pin__is_set__impl
#define set__impl G2_unused__pin__set__impl

G2_unused__pin_t const G2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl G2_unused__pin__is_set__impl
#define set__impl G2_unused__pin__set__impl


;

#define is_set__impl G2_unused__pin__is_set__impl
#define set__impl G2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void G2_unused__init() {
    G2_unused__ddr.set(0);
    G2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} G2_unused_t;

extern G2_unused_t const G2_unused;

static AKAT_FORCE_INLINE u8 G2_unused__is_set__impl() {
#define is_set__impl G2_unused__is_set__impl
    return G2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl G2_unused__is_set__impl

G2_unused_t const G2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl G2_unused__is_set__impl


;

#define is_set__impl G2_unused__is_set__impl




#undef is_set__impl
;



;
; // 70   PG2 ( ALE ) Digital pin 39
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A7_unused__port_t;

extern A7_unused__port_t const A7_unused__port;

static AKAT_FORCE_INLINE void A7_unused__port__set__impl(u8 state) {
#define set__impl A7_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 7;  //Set PORTA of A7 to 1
    } else {
        PORTA &= ~(1 << 7);  //Set PORTA of A7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A7_unused__port__is_set__impl() {
#define is_set__impl A7_unused__port__is_set__impl
#define set__impl A7_unused__port__set__impl
    return PORTA & (1 << 7);  //Get value of PORTA for A7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A7_unused__port__is_set__impl
#define set__impl A7_unused__port__set__impl

A7_unused__port_t const A7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A7_unused__port__is_set__impl
#define set__impl A7_unused__port__set__impl


;

#define is_set__impl A7_unused__port__is_set__impl
#define set__impl A7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A7_unused__ddr_t;

extern A7_unused__ddr_t const A7_unused__ddr;

static AKAT_FORCE_INLINE void A7_unused__ddr__set__impl(u8 state) {
#define set__impl A7_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 7;  //Set DDRA of A7 to 1
    } else {
        DDRA &= ~(1 << 7);  //Set DDRA of A7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A7_unused__ddr__is_set__impl() {
#define is_set__impl A7_unused__ddr__is_set__impl
#define set__impl A7_unused__ddr__set__impl
    return DDRA & (1 << 7);  //Get value of DDRA for A7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A7_unused__ddr__is_set__impl
#define set__impl A7_unused__ddr__set__impl

A7_unused__ddr_t const A7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A7_unused__ddr__is_set__impl
#define set__impl A7_unused__ddr__set__impl


;

#define is_set__impl A7_unused__ddr__is_set__impl
#define set__impl A7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A7_unused__pin_t;

extern A7_unused__pin_t const A7_unused__pin;

static AKAT_FORCE_INLINE void A7_unused__pin__set__impl(u8 state) {
#define set__impl A7_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 7;  //Set PINA of A7 to 1
    } else {
        PINA &= ~(1 << 7);  //Set PINA of A7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A7_unused__pin__is_set__impl() {
#define is_set__impl A7_unused__pin__is_set__impl
#define set__impl A7_unused__pin__set__impl
    return PINA & (1 << 7);  //Get value of PINA for A7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A7_unused__pin__is_set__impl
#define set__impl A7_unused__pin__set__impl

A7_unused__pin_t const A7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A7_unused__pin__is_set__impl
#define set__impl A7_unused__pin__set__impl


;

#define is_set__impl A7_unused__pin__is_set__impl
#define set__impl A7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A7_unused__init() {
    A7_unused__ddr.set(0);
    A7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A7_unused_t;

extern A7_unused_t const A7_unused;

static AKAT_FORCE_INLINE u8 A7_unused__is_set__impl() {
#define is_set__impl A7_unused__is_set__impl
    return A7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A7_unused__is_set__impl

A7_unused_t const A7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A7_unused__is_set__impl


;

#define is_set__impl A7_unused__is_set__impl




#undef is_set__impl
;



;
; // 71   PA7 ( AD7 ) Digital pin 29
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A6_unused__port_t;

extern A6_unused__port_t const A6_unused__port;

static AKAT_FORCE_INLINE void A6_unused__port__set__impl(u8 state) {
#define set__impl A6_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 6;  //Set PORTA of A6 to 1
    } else {
        PORTA &= ~(1 << 6);  //Set PORTA of A6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A6_unused__port__is_set__impl() {
#define is_set__impl A6_unused__port__is_set__impl
#define set__impl A6_unused__port__set__impl
    return PORTA & (1 << 6);  //Get value of PORTA for A6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A6_unused__port__is_set__impl
#define set__impl A6_unused__port__set__impl

A6_unused__port_t const A6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A6_unused__port__is_set__impl
#define set__impl A6_unused__port__set__impl


;

#define is_set__impl A6_unused__port__is_set__impl
#define set__impl A6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A6_unused__ddr_t;

extern A6_unused__ddr_t const A6_unused__ddr;

static AKAT_FORCE_INLINE void A6_unused__ddr__set__impl(u8 state) {
#define set__impl A6_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 6;  //Set DDRA of A6 to 1
    } else {
        DDRA &= ~(1 << 6);  //Set DDRA of A6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A6_unused__ddr__is_set__impl() {
#define is_set__impl A6_unused__ddr__is_set__impl
#define set__impl A6_unused__ddr__set__impl
    return DDRA & (1 << 6);  //Get value of DDRA for A6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A6_unused__ddr__is_set__impl
#define set__impl A6_unused__ddr__set__impl

A6_unused__ddr_t const A6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A6_unused__ddr__is_set__impl
#define set__impl A6_unused__ddr__set__impl


;

#define is_set__impl A6_unused__ddr__is_set__impl
#define set__impl A6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A6_unused__pin_t;

extern A6_unused__pin_t const A6_unused__pin;

static AKAT_FORCE_INLINE void A6_unused__pin__set__impl(u8 state) {
#define set__impl A6_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 6;  //Set PINA of A6 to 1
    } else {
        PINA &= ~(1 << 6);  //Set PINA of A6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A6_unused__pin__is_set__impl() {
#define is_set__impl A6_unused__pin__is_set__impl
#define set__impl A6_unused__pin__set__impl
    return PINA & (1 << 6);  //Get value of PINA for A6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A6_unused__pin__is_set__impl
#define set__impl A6_unused__pin__set__impl

A6_unused__pin_t const A6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A6_unused__pin__is_set__impl
#define set__impl A6_unused__pin__set__impl


;

#define is_set__impl A6_unused__pin__is_set__impl
#define set__impl A6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A6_unused__init() {
    A6_unused__ddr.set(0);
    A6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A6_unused_t;

extern A6_unused_t const A6_unused;

static AKAT_FORCE_INLINE u8 A6_unused__is_set__impl() {
#define is_set__impl A6_unused__is_set__impl
    return A6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A6_unused__is_set__impl

A6_unused_t const A6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A6_unused__is_set__impl


;

#define is_set__impl A6_unused__is_set__impl




#undef is_set__impl
;



;
; // 72   PA6 ( AD6 ) Digital pin 28
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A5_unused__port_t;

extern A5_unused__port_t const A5_unused__port;

static AKAT_FORCE_INLINE void A5_unused__port__set__impl(u8 state) {
#define set__impl A5_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 5;  //Set PORTA of A5 to 1
    } else {
        PORTA &= ~(1 << 5);  //Set PORTA of A5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A5_unused__port__is_set__impl() {
#define is_set__impl A5_unused__port__is_set__impl
#define set__impl A5_unused__port__set__impl
    return PORTA & (1 << 5);  //Get value of PORTA for A5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A5_unused__port__is_set__impl
#define set__impl A5_unused__port__set__impl

A5_unused__port_t const A5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A5_unused__port__is_set__impl
#define set__impl A5_unused__port__set__impl


;

#define is_set__impl A5_unused__port__is_set__impl
#define set__impl A5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A5_unused__ddr_t;

extern A5_unused__ddr_t const A5_unused__ddr;

static AKAT_FORCE_INLINE void A5_unused__ddr__set__impl(u8 state) {
#define set__impl A5_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 5;  //Set DDRA of A5 to 1
    } else {
        DDRA &= ~(1 << 5);  //Set DDRA of A5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A5_unused__ddr__is_set__impl() {
#define is_set__impl A5_unused__ddr__is_set__impl
#define set__impl A5_unused__ddr__set__impl
    return DDRA & (1 << 5);  //Get value of DDRA for A5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A5_unused__ddr__is_set__impl
#define set__impl A5_unused__ddr__set__impl

A5_unused__ddr_t const A5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A5_unused__ddr__is_set__impl
#define set__impl A5_unused__ddr__set__impl


;

#define is_set__impl A5_unused__ddr__is_set__impl
#define set__impl A5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A5_unused__pin_t;

extern A5_unused__pin_t const A5_unused__pin;

static AKAT_FORCE_INLINE void A5_unused__pin__set__impl(u8 state) {
#define set__impl A5_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 5;  //Set PINA of A5 to 1
    } else {
        PINA &= ~(1 << 5);  //Set PINA of A5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A5_unused__pin__is_set__impl() {
#define is_set__impl A5_unused__pin__is_set__impl
#define set__impl A5_unused__pin__set__impl
    return PINA & (1 << 5);  //Get value of PINA for A5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A5_unused__pin__is_set__impl
#define set__impl A5_unused__pin__set__impl

A5_unused__pin_t const A5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A5_unused__pin__is_set__impl
#define set__impl A5_unused__pin__set__impl


;

#define is_set__impl A5_unused__pin__is_set__impl
#define set__impl A5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A5_unused__init() {
    A5_unused__ddr.set(0);
    A5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A5_unused_t;

extern A5_unused_t const A5_unused;

static AKAT_FORCE_INLINE u8 A5_unused__is_set__impl() {
#define is_set__impl A5_unused__is_set__impl
    return A5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A5_unused__is_set__impl

A5_unused_t const A5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A5_unused__is_set__impl


;

#define is_set__impl A5_unused__is_set__impl




#undef is_set__impl
;



;
; // 73   PA5 ( AD5 ) Digital pin 27
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A4_unused__port_t;

extern A4_unused__port_t const A4_unused__port;

static AKAT_FORCE_INLINE void A4_unused__port__set__impl(u8 state) {
#define set__impl A4_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 4;  //Set PORTA of A4 to 1
    } else {
        PORTA &= ~(1 << 4);  //Set PORTA of A4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A4_unused__port__is_set__impl() {
#define is_set__impl A4_unused__port__is_set__impl
#define set__impl A4_unused__port__set__impl
    return PORTA & (1 << 4);  //Get value of PORTA for A4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A4_unused__port__is_set__impl
#define set__impl A4_unused__port__set__impl

A4_unused__port_t const A4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A4_unused__port__is_set__impl
#define set__impl A4_unused__port__set__impl


;

#define is_set__impl A4_unused__port__is_set__impl
#define set__impl A4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A4_unused__ddr_t;

extern A4_unused__ddr_t const A4_unused__ddr;

static AKAT_FORCE_INLINE void A4_unused__ddr__set__impl(u8 state) {
#define set__impl A4_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 4;  //Set DDRA of A4 to 1
    } else {
        DDRA &= ~(1 << 4);  //Set DDRA of A4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A4_unused__ddr__is_set__impl() {
#define is_set__impl A4_unused__ddr__is_set__impl
#define set__impl A4_unused__ddr__set__impl
    return DDRA & (1 << 4);  //Get value of DDRA for A4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A4_unused__ddr__is_set__impl
#define set__impl A4_unused__ddr__set__impl

A4_unused__ddr_t const A4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A4_unused__ddr__is_set__impl
#define set__impl A4_unused__ddr__set__impl


;

#define is_set__impl A4_unused__ddr__is_set__impl
#define set__impl A4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A4_unused__pin_t;

extern A4_unused__pin_t const A4_unused__pin;

static AKAT_FORCE_INLINE void A4_unused__pin__set__impl(u8 state) {
#define set__impl A4_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 4;  //Set PINA of A4 to 1
    } else {
        PINA &= ~(1 << 4);  //Set PINA of A4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A4_unused__pin__is_set__impl() {
#define is_set__impl A4_unused__pin__is_set__impl
#define set__impl A4_unused__pin__set__impl
    return PINA & (1 << 4);  //Get value of PINA for A4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A4_unused__pin__is_set__impl
#define set__impl A4_unused__pin__set__impl

A4_unused__pin_t const A4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A4_unused__pin__is_set__impl
#define set__impl A4_unused__pin__set__impl


;

#define is_set__impl A4_unused__pin__is_set__impl
#define set__impl A4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A4_unused__init() {
    A4_unused__ddr.set(0);
    A4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A4_unused_t;

extern A4_unused_t const A4_unused;

static AKAT_FORCE_INLINE u8 A4_unused__is_set__impl() {
#define is_set__impl A4_unused__is_set__impl
    return A4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A4_unused__is_set__impl

A4_unused_t const A4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A4_unused__is_set__impl


;

#define is_set__impl A4_unused__is_set__impl




#undef is_set__impl
;



;
; // 74   PA4 ( AD4 ) Digital pin 26
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A3_unused__port_t;

extern A3_unused__port_t const A3_unused__port;

static AKAT_FORCE_INLINE void A3_unused__port__set__impl(u8 state) {
#define set__impl A3_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 3;  //Set PORTA of A3 to 1
    } else {
        PORTA &= ~(1 << 3);  //Set PORTA of A3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A3_unused__port__is_set__impl() {
#define is_set__impl A3_unused__port__is_set__impl
#define set__impl A3_unused__port__set__impl
    return PORTA & (1 << 3);  //Get value of PORTA for A3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A3_unused__port__is_set__impl
#define set__impl A3_unused__port__set__impl

A3_unused__port_t const A3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A3_unused__port__is_set__impl
#define set__impl A3_unused__port__set__impl


;

#define is_set__impl A3_unused__port__is_set__impl
#define set__impl A3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A3_unused__ddr_t;

extern A3_unused__ddr_t const A3_unused__ddr;

static AKAT_FORCE_INLINE void A3_unused__ddr__set__impl(u8 state) {
#define set__impl A3_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 3;  //Set DDRA of A3 to 1
    } else {
        DDRA &= ~(1 << 3);  //Set DDRA of A3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A3_unused__ddr__is_set__impl() {
#define is_set__impl A3_unused__ddr__is_set__impl
#define set__impl A3_unused__ddr__set__impl
    return DDRA & (1 << 3);  //Get value of DDRA for A3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A3_unused__ddr__is_set__impl
#define set__impl A3_unused__ddr__set__impl

A3_unused__ddr_t const A3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A3_unused__ddr__is_set__impl
#define set__impl A3_unused__ddr__set__impl


;

#define is_set__impl A3_unused__ddr__is_set__impl
#define set__impl A3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A3_unused__pin_t;

extern A3_unused__pin_t const A3_unused__pin;

static AKAT_FORCE_INLINE void A3_unused__pin__set__impl(u8 state) {
#define set__impl A3_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 3;  //Set PINA of A3 to 1
    } else {
        PINA &= ~(1 << 3);  //Set PINA of A3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A3_unused__pin__is_set__impl() {
#define is_set__impl A3_unused__pin__is_set__impl
#define set__impl A3_unused__pin__set__impl
    return PINA & (1 << 3);  //Get value of PINA for A3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A3_unused__pin__is_set__impl
#define set__impl A3_unused__pin__set__impl

A3_unused__pin_t const A3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A3_unused__pin__is_set__impl
#define set__impl A3_unused__pin__set__impl


;

#define is_set__impl A3_unused__pin__is_set__impl
#define set__impl A3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A3_unused__init() {
    A3_unused__ddr.set(0);
    A3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A3_unused_t;

extern A3_unused_t const A3_unused;

static AKAT_FORCE_INLINE u8 A3_unused__is_set__impl() {
#define is_set__impl A3_unused__is_set__impl
    return A3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A3_unused__is_set__impl

A3_unused_t const A3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A3_unused__is_set__impl


;

#define is_set__impl A3_unused__is_set__impl




#undef is_set__impl
;



;
; // 75   PA3 ( AD3 ) Digital pin 25
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A2_unused__port_t;

extern A2_unused__port_t const A2_unused__port;

static AKAT_FORCE_INLINE void A2_unused__port__set__impl(u8 state) {
#define set__impl A2_unused__port__set__impl

    if (state) {
        PORTA |= 1 << 2;  //Set PORTA of A2 to 1
    } else {
        PORTA &= ~(1 << 2);  //Set PORTA of A2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A2_unused__port__is_set__impl() {
#define is_set__impl A2_unused__port__is_set__impl
#define set__impl A2_unused__port__set__impl
    return PORTA & (1 << 2);  //Get value of PORTA for A2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A2_unused__port__is_set__impl
#define set__impl A2_unused__port__set__impl

A2_unused__port_t const A2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl A2_unused__port__is_set__impl
#define set__impl A2_unused__port__set__impl


;

#define is_set__impl A2_unused__port__is_set__impl
#define set__impl A2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A2_unused__ddr_t;

extern A2_unused__ddr_t const A2_unused__ddr;

static AKAT_FORCE_INLINE void A2_unused__ddr__set__impl(u8 state) {
#define set__impl A2_unused__ddr__set__impl

    if (state) {
        DDRA |= 1 << 2;  //Set DDRA of A2 to 1
    } else {
        DDRA &= ~(1 << 2);  //Set DDRA of A2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A2_unused__ddr__is_set__impl() {
#define is_set__impl A2_unused__ddr__is_set__impl
#define set__impl A2_unused__ddr__set__impl
    return DDRA & (1 << 2);  //Get value of DDRA for A2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A2_unused__ddr__is_set__impl
#define set__impl A2_unused__ddr__set__impl

A2_unused__ddr_t const A2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A2_unused__ddr__is_set__impl
#define set__impl A2_unused__ddr__set__impl


;

#define is_set__impl A2_unused__ddr__is_set__impl
#define set__impl A2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} A2_unused__pin_t;

extern A2_unused__pin_t const A2_unused__pin;

static AKAT_FORCE_INLINE void A2_unused__pin__set__impl(u8 state) {
#define set__impl A2_unused__pin__set__impl

    if (state) {
        PINA |= 1 << 2;  //Set PINA of A2 to 1
    } else {
        PINA &= ~(1 << 2);  //Set PINA of A2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 A2_unused__pin__is_set__impl() {
#define is_set__impl A2_unused__pin__is_set__impl
#define set__impl A2_unused__pin__set__impl
    return PINA & (1 << 2);  //Get value of PINA for A2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl A2_unused__pin__is_set__impl
#define set__impl A2_unused__pin__set__impl

A2_unused__pin_t const A2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl A2_unused__pin__is_set__impl
#define set__impl A2_unused__pin__set__impl


;

#define is_set__impl A2_unused__pin__is_set__impl
#define set__impl A2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void A2_unused__init() {
    A2_unused__ddr.set(0);
    A2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} A2_unused_t;

extern A2_unused_t const A2_unused;

static AKAT_FORCE_INLINE u8 A2_unused__is_set__impl() {
#define is_set__impl A2_unused__is_set__impl
    return A2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl A2_unused__is_set__impl

A2_unused_t const A2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl A2_unused__is_set__impl


;

#define is_set__impl A2_unused__is_set__impl




#undef is_set__impl
;



;
; // 76   PA2 ( AD2 ) Digital pin 24
// DS18B20 Case ..... 77   PA1 ( AD1 ) Digital pin 23
// DS18B20 Aqua ..... 78   PA0 ( AD0 ) Digital pin 22
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J7_unused__port_t;

extern J7_unused__port_t const J7_unused__port;

static AKAT_FORCE_INLINE void J7_unused__port__set__impl(u8 state) {
#define set__impl J7_unused__port__set__impl

    if (state) {
        PORTJ |= 1 << 7;  //Set PORTJ of J7 to 1
    } else {
        PORTJ &= ~(1 << 7);  //Set PORTJ of J7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J7_unused__port__is_set__impl() {
#define is_set__impl J7_unused__port__is_set__impl
#define set__impl J7_unused__port__set__impl
    return PORTJ & (1 << 7);  //Get value of PORTJ for J7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J7_unused__port__is_set__impl
#define set__impl J7_unused__port__set__impl

J7_unused__port_t const J7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl J7_unused__port__is_set__impl
#define set__impl J7_unused__port__set__impl


;

#define is_set__impl J7_unused__port__is_set__impl
#define set__impl J7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J7_unused__ddr_t;

extern J7_unused__ddr_t const J7_unused__ddr;

static AKAT_FORCE_INLINE void J7_unused__ddr__set__impl(u8 state) {
#define set__impl J7_unused__ddr__set__impl

    if (state) {
        DDRJ |= 1 << 7;  //Set DDRJ of J7 to 1
    } else {
        DDRJ &= ~(1 << 7);  //Set DDRJ of J7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J7_unused__ddr__is_set__impl() {
#define is_set__impl J7_unused__ddr__is_set__impl
#define set__impl J7_unused__ddr__set__impl
    return DDRJ & (1 << 7);  //Get value of DDRJ for J7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J7_unused__ddr__is_set__impl
#define set__impl J7_unused__ddr__set__impl

J7_unused__ddr_t const J7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J7_unused__ddr__is_set__impl
#define set__impl J7_unused__ddr__set__impl


;

#define is_set__impl J7_unused__ddr__is_set__impl
#define set__impl J7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} J7_unused__pin_t;

extern J7_unused__pin_t const J7_unused__pin;

static AKAT_FORCE_INLINE void J7_unused__pin__set__impl(u8 state) {
#define set__impl J7_unused__pin__set__impl

    if (state) {
        PINJ |= 1 << 7;  //Set PINJ of J7 to 1
    } else {
        PINJ &= ~(1 << 7);  //Set PINJ of J7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 J7_unused__pin__is_set__impl() {
#define is_set__impl J7_unused__pin__is_set__impl
#define set__impl J7_unused__pin__set__impl
    return PINJ & (1 << 7);  //Get value of PINJ for J7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl J7_unused__pin__is_set__impl
#define set__impl J7_unused__pin__set__impl

J7_unused__pin_t const J7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl J7_unused__pin__is_set__impl
#define set__impl J7_unused__pin__set__impl


;

#define is_set__impl J7_unused__pin__is_set__impl
#define set__impl J7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void J7_unused__init() {
    J7_unused__ddr.set(0);
    J7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} J7_unused_t;

extern J7_unused_t const J7_unused;

static AKAT_FORCE_INLINE u8 J7_unused__is_set__impl() {
#define is_set__impl J7_unused__is_set__impl
    return J7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl J7_unused__is_set__impl

J7_unused_t const J7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl J7_unused__is_set__impl


;

#define is_set__impl J7_unused__is_set__impl




#undef is_set__impl
;



;
; // 79   PJ7
// .................. 80   VCC
// .................. 81   GND
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K7_unused__port_t;

extern K7_unused__port_t const K7_unused__port;

static AKAT_FORCE_INLINE void K7_unused__port__set__impl(u8 state) {
#define set__impl K7_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 7;  //Set PORTK of K7 to 1
    } else {
        PORTK &= ~(1 << 7);  //Set PORTK of K7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K7_unused__port__is_set__impl() {
#define is_set__impl K7_unused__port__is_set__impl
#define set__impl K7_unused__port__set__impl
    return PORTK & (1 << 7);  //Get value of PORTK for K7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K7_unused__port__is_set__impl
#define set__impl K7_unused__port__set__impl

K7_unused__port_t const K7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K7_unused__port__is_set__impl
#define set__impl K7_unused__port__set__impl


;

#define is_set__impl K7_unused__port__is_set__impl
#define set__impl K7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K7_unused__ddr_t;

extern K7_unused__ddr_t const K7_unused__ddr;

static AKAT_FORCE_INLINE void K7_unused__ddr__set__impl(u8 state) {
#define set__impl K7_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 7;  //Set DDRK of K7 to 1
    } else {
        DDRK &= ~(1 << 7);  //Set DDRK of K7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K7_unused__ddr__is_set__impl() {
#define is_set__impl K7_unused__ddr__is_set__impl
#define set__impl K7_unused__ddr__set__impl
    return DDRK & (1 << 7);  //Get value of DDRK for K7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K7_unused__ddr__is_set__impl
#define set__impl K7_unused__ddr__set__impl

K7_unused__ddr_t const K7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K7_unused__ddr__is_set__impl
#define set__impl K7_unused__ddr__set__impl


;

#define is_set__impl K7_unused__ddr__is_set__impl
#define set__impl K7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K7_unused__pin_t;

extern K7_unused__pin_t const K7_unused__pin;

static AKAT_FORCE_INLINE void K7_unused__pin__set__impl(u8 state) {
#define set__impl K7_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 7;  //Set PINK of K7 to 1
    } else {
        PINK &= ~(1 << 7);  //Set PINK of K7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K7_unused__pin__is_set__impl() {
#define is_set__impl K7_unused__pin__is_set__impl
#define set__impl K7_unused__pin__set__impl
    return PINK & (1 << 7);  //Get value of PINK for K7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K7_unused__pin__is_set__impl
#define set__impl K7_unused__pin__set__impl

K7_unused__pin_t const K7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K7_unused__pin__is_set__impl
#define set__impl K7_unused__pin__set__impl


;

#define is_set__impl K7_unused__pin__is_set__impl
#define set__impl K7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K7_unused__init() {
    K7_unused__ddr.set(0);
    K7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K7_unused_t;

extern K7_unused_t const K7_unused;

static AKAT_FORCE_INLINE u8 K7_unused__is_set__impl() {
#define is_set__impl K7_unused__is_set__impl
    return K7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K7_unused__is_set__impl

K7_unused_t const K7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K7_unused__is_set__impl


;

#define is_set__impl K7_unused__is_set__impl




#undef is_set__impl
;



;
; // 82   PK7 ( ADC15/PCINT23 ) Analog pin 15
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K6_unused__port_t;

extern K6_unused__port_t const K6_unused__port;

static AKAT_FORCE_INLINE void K6_unused__port__set__impl(u8 state) {
#define set__impl K6_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 6;  //Set PORTK of K6 to 1
    } else {
        PORTK &= ~(1 << 6);  //Set PORTK of K6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K6_unused__port__is_set__impl() {
#define is_set__impl K6_unused__port__is_set__impl
#define set__impl K6_unused__port__set__impl
    return PORTK & (1 << 6);  //Get value of PORTK for K6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K6_unused__port__is_set__impl
#define set__impl K6_unused__port__set__impl

K6_unused__port_t const K6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K6_unused__port__is_set__impl
#define set__impl K6_unused__port__set__impl


;

#define is_set__impl K6_unused__port__is_set__impl
#define set__impl K6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K6_unused__ddr_t;

extern K6_unused__ddr_t const K6_unused__ddr;

static AKAT_FORCE_INLINE void K6_unused__ddr__set__impl(u8 state) {
#define set__impl K6_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 6;  //Set DDRK of K6 to 1
    } else {
        DDRK &= ~(1 << 6);  //Set DDRK of K6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K6_unused__ddr__is_set__impl() {
#define is_set__impl K6_unused__ddr__is_set__impl
#define set__impl K6_unused__ddr__set__impl
    return DDRK & (1 << 6);  //Get value of DDRK for K6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K6_unused__ddr__is_set__impl
#define set__impl K6_unused__ddr__set__impl

K6_unused__ddr_t const K6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K6_unused__ddr__is_set__impl
#define set__impl K6_unused__ddr__set__impl


;

#define is_set__impl K6_unused__ddr__is_set__impl
#define set__impl K6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K6_unused__pin_t;

extern K6_unused__pin_t const K6_unused__pin;

static AKAT_FORCE_INLINE void K6_unused__pin__set__impl(u8 state) {
#define set__impl K6_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 6;  //Set PINK of K6 to 1
    } else {
        PINK &= ~(1 << 6);  //Set PINK of K6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K6_unused__pin__is_set__impl() {
#define is_set__impl K6_unused__pin__is_set__impl
#define set__impl K6_unused__pin__set__impl
    return PINK & (1 << 6);  //Get value of PINK for K6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K6_unused__pin__is_set__impl
#define set__impl K6_unused__pin__set__impl

K6_unused__pin_t const K6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K6_unused__pin__is_set__impl
#define set__impl K6_unused__pin__set__impl


;

#define is_set__impl K6_unused__pin__is_set__impl
#define set__impl K6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K6_unused__init() {
    K6_unused__ddr.set(0);
    K6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K6_unused_t;

extern K6_unused_t const K6_unused;

static AKAT_FORCE_INLINE u8 K6_unused__is_set__impl() {
#define is_set__impl K6_unused__is_set__impl
    return K6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K6_unused__is_set__impl

K6_unused_t const K6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K6_unused__is_set__impl


;

#define is_set__impl K6_unused__is_set__impl




#undef is_set__impl
;



;
; // 83   PK6 ( ADC14/PCINT22 ) Analog pin 14
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K5_unused__port_t;

extern K5_unused__port_t const K5_unused__port;

static AKAT_FORCE_INLINE void K5_unused__port__set__impl(u8 state) {
#define set__impl K5_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 5;  //Set PORTK of K5 to 1
    } else {
        PORTK &= ~(1 << 5);  //Set PORTK of K5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K5_unused__port__is_set__impl() {
#define is_set__impl K5_unused__port__is_set__impl
#define set__impl K5_unused__port__set__impl
    return PORTK & (1 << 5);  //Get value of PORTK for K5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K5_unused__port__is_set__impl
#define set__impl K5_unused__port__set__impl

K5_unused__port_t const K5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K5_unused__port__is_set__impl
#define set__impl K5_unused__port__set__impl


;

#define is_set__impl K5_unused__port__is_set__impl
#define set__impl K5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K5_unused__ddr_t;

extern K5_unused__ddr_t const K5_unused__ddr;

static AKAT_FORCE_INLINE void K5_unused__ddr__set__impl(u8 state) {
#define set__impl K5_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 5;  //Set DDRK of K5 to 1
    } else {
        DDRK &= ~(1 << 5);  //Set DDRK of K5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K5_unused__ddr__is_set__impl() {
#define is_set__impl K5_unused__ddr__is_set__impl
#define set__impl K5_unused__ddr__set__impl
    return DDRK & (1 << 5);  //Get value of DDRK for K5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K5_unused__ddr__is_set__impl
#define set__impl K5_unused__ddr__set__impl

K5_unused__ddr_t const K5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K5_unused__ddr__is_set__impl
#define set__impl K5_unused__ddr__set__impl


;

#define is_set__impl K5_unused__ddr__is_set__impl
#define set__impl K5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K5_unused__pin_t;

extern K5_unused__pin_t const K5_unused__pin;

static AKAT_FORCE_INLINE void K5_unused__pin__set__impl(u8 state) {
#define set__impl K5_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 5;  //Set PINK of K5 to 1
    } else {
        PINK &= ~(1 << 5);  //Set PINK of K5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K5_unused__pin__is_set__impl() {
#define is_set__impl K5_unused__pin__is_set__impl
#define set__impl K5_unused__pin__set__impl
    return PINK & (1 << 5);  //Get value of PINK for K5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K5_unused__pin__is_set__impl
#define set__impl K5_unused__pin__set__impl

K5_unused__pin_t const K5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K5_unused__pin__is_set__impl
#define set__impl K5_unused__pin__set__impl


;

#define is_set__impl K5_unused__pin__is_set__impl
#define set__impl K5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K5_unused__init() {
    K5_unused__ddr.set(0);
    K5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K5_unused_t;

extern K5_unused_t const K5_unused;

static AKAT_FORCE_INLINE u8 K5_unused__is_set__impl() {
#define is_set__impl K5_unused__is_set__impl
    return K5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K5_unused__is_set__impl

K5_unused_t const K5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K5_unused__is_set__impl


;

#define is_set__impl K5_unused__is_set__impl




#undef is_set__impl
;



;
; // 84   PK5 ( ADC13/PCINT21 ) Analog pin 13
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K4_unused__port_t;

extern K4_unused__port_t const K4_unused__port;

static AKAT_FORCE_INLINE void K4_unused__port__set__impl(u8 state) {
#define set__impl K4_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 4;  //Set PORTK of K4 to 1
    } else {
        PORTK &= ~(1 << 4);  //Set PORTK of K4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K4_unused__port__is_set__impl() {
#define is_set__impl K4_unused__port__is_set__impl
#define set__impl K4_unused__port__set__impl
    return PORTK & (1 << 4);  //Get value of PORTK for K4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K4_unused__port__is_set__impl
#define set__impl K4_unused__port__set__impl

K4_unused__port_t const K4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K4_unused__port__is_set__impl
#define set__impl K4_unused__port__set__impl


;

#define is_set__impl K4_unused__port__is_set__impl
#define set__impl K4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K4_unused__ddr_t;

extern K4_unused__ddr_t const K4_unused__ddr;

static AKAT_FORCE_INLINE void K4_unused__ddr__set__impl(u8 state) {
#define set__impl K4_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 4;  //Set DDRK of K4 to 1
    } else {
        DDRK &= ~(1 << 4);  //Set DDRK of K4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K4_unused__ddr__is_set__impl() {
#define is_set__impl K4_unused__ddr__is_set__impl
#define set__impl K4_unused__ddr__set__impl
    return DDRK & (1 << 4);  //Get value of DDRK for K4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K4_unused__ddr__is_set__impl
#define set__impl K4_unused__ddr__set__impl

K4_unused__ddr_t const K4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K4_unused__ddr__is_set__impl
#define set__impl K4_unused__ddr__set__impl


;

#define is_set__impl K4_unused__ddr__is_set__impl
#define set__impl K4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K4_unused__pin_t;

extern K4_unused__pin_t const K4_unused__pin;

static AKAT_FORCE_INLINE void K4_unused__pin__set__impl(u8 state) {
#define set__impl K4_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 4;  //Set PINK of K4 to 1
    } else {
        PINK &= ~(1 << 4);  //Set PINK of K4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K4_unused__pin__is_set__impl() {
#define is_set__impl K4_unused__pin__is_set__impl
#define set__impl K4_unused__pin__set__impl
    return PINK & (1 << 4);  //Get value of PINK for K4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K4_unused__pin__is_set__impl
#define set__impl K4_unused__pin__set__impl

K4_unused__pin_t const K4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K4_unused__pin__is_set__impl
#define set__impl K4_unused__pin__set__impl


;

#define is_set__impl K4_unused__pin__is_set__impl
#define set__impl K4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K4_unused__init() {
    K4_unused__ddr.set(0);
    K4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K4_unused_t;

extern K4_unused_t const K4_unused;

static AKAT_FORCE_INLINE u8 K4_unused__is_set__impl() {
#define is_set__impl K4_unused__is_set__impl
    return K4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K4_unused__is_set__impl

K4_unused_t const K4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K4_unused__is_set__impl


;

#define is_set__impl K4_unused__is_set__impl




#undef is_set__impl
;



;
; // 85   PK4 ( ADC12/PCINT20 ) Analog pin 12
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K3_unused__port_t;

extern K3_unused__port_t const K3_unused__port;

static AKAT_FORCE_INLINE void K3_unused__port__set__impl(u8 state) {
#define set__impl K3_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 3;  //Set PORTK of K3 to 1
    } else {
        PORTK &= ~(1 << 3);  //Set PORTK of K3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K3_unused__port__is_set__impl() {
#define is_set__impl K3_unused__port__is_set__impl
#define set__impl K3_unused__port__set__impl
    return PORTK & (1 << 3);  //Get value of PORTK for K3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K3_unused__port__is_set__impl
#define set__impl K3_unused__port__set__impl

K3_unused__port_t const K3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K3_unused__port__is_set__impl
#define set__impl K3_unused__port__set__impl


;

#define is_set__impl K3_unused__port__is_set__impl
#define set__impl K3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K3_unused__ddr_t;

extern K3_unused__ddr_t const K3_unused__ddr;

static AKAT_FORCE_INLINE void K3_unused__ddr__set__impl(u8 state) {
#define set__impl K3_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 3;  //Set DDRK of K3 to 1
    } else {
        DDRK &= ~(1 << 3);  //Set DDRK of K3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K3_unused__ddr__is_set__impl() {
#define is_set__impl K3_unused__ddr__is_set__impl
#define set__impl K3_unused__ddr__set__impl
    return DDRK & (1 << 3);  //Get value of DDRK for K3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K3_unused__ddr__is_set__impl
#define set__impl K3_unused__ddr__set__impl

K3_unused__ddr_t const K3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K3_unused__ddr__is_set__impl
#define set__impl K3_unused__ddr__set__impl


;

#define is_set__impl K3_unused__ddr__is_set__impl
#define set__impl K3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K3_unused__pin_t;

extern K3_unused__pin_t const K3_unused__pin;

static AKAT_FORCE_INLINE void K3_unused__pin__set__impl(u8 state) {
#define set__impl K3_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 3;  //Set PINK of K3 to 1
    } else {
        PINK &= ~(1 << 3);  //Set PINK of K3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K3_unused__pin__is_set__impl() {
#define is_set__impl K3_unused__pin__is_set__impl
#define set__impl K3_unused__pin__set__impl
    return PINK & (1 << 3);  //Get value of PINK for K3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K3_unused__pin__is_set__impl
#define set__impl K3_unused__pin__set__impl

K3_unused__pin_t const K3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K3_unused__pin__is_set__impl
#define set__impl K3_unused__pin__set__impl


;

#define is_set__impl K3_unused__pin__is_set__impl
#define set__impl K3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K3_unused__init() {
    K3_unused__ddr.set(0);
    K3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K3_unused_t;

extern K3_unused_t const K3_unused;

static AKAT_FORCE_INLINE u8 K3_unused__is_set__impl() {
#define is_set__impl K3_unused__is_set__impl
    return K3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K3_unused__is_set__impl

K3_unused_t const K3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K3_unused__is_set__impl


;

#define is_set__impl K3_unused__is_set__impl




#undef is_set__impl
;



;
; // 86   PK3 ( ADC11/PCINT19 ) Analog pin 11
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K2_unused__port_t;

extern K2_unused__port_t const K2_unused__port;

static AKAT_FORCE_INLINE void K2_unused__port__set__impl(u8 state) {
#define set__impl K2_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 2;  //Set PORTK of K2 to 1
    } else {
        PORTK &= ~(1 << 2);  //Set PORTK of K2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K2_unused__port__is_set__impl() {
#define is_set__impl K2_unused__port__is_set__impl
#define set__impl K2_unused__port__set__impl
    return PORTK & (1 << 2);  //Get value of PORTK for K2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K2_unused__port__is_set__impl
#define set__impl K2_unused__port__set__impl

K2_unused__port_t const K2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K2_unused__port__is_set__impl
#define set__impl K2_unused__port__set__impl


;

#define is_set__impl K2_unused__port__is_set__impl
#define set__impl K2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K2_unused__ddr_t;

extern K2_unused__ddr_t const K2_unused__ddr;

static AKAT_FORCE_INLINE void K2_unused__ddr__set__impl(u8 state) {
#define set__impl K2_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 2;  //Set DDRK of K2 to 1
    } else {
        DDRK &= ~(1 << 2);  //Set DDRK of K2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K2_unused__ddr__is_set__impl() {
#define is_set__impl K2_unused__ddr__is_set__impl
#define set__impl K2_unused__ddr__set__impl
    return DDRK & (1 << 2);  //Get value of DDRK for K2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K2_unused__ddr__is_set__impl
#define set__impl K2_unused__ddr__set__impl

K2_unused__ddr_t const K2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K2_unused__ddr__is_set__impl
#define set__impl K2_unused__ddr__set__impl


;

#define is_set__impl K2_unused__ddr__is_set__impl
#define set__impl K2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K2_unused__pin_t;

extern K2_unused__pin_t const K2_unused__pin;

static AKAT_FORCE_INLINE void K2_unused__pin__set__impl(u8 state) {
#define set__impl K2_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 2;  //Set PINK of K2 to 1
    } else {
        PINK &= ~(1 << 2);  //Set PINK of K2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K2_unused__pin__is_set__impl() {
#define is_set__impl K2_unused__pin__is_set__impl
#define set__impl K2_unused__pin__set__impl
    return PINK & (1 << 2);  //Get value of PINK for K2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K2_unused__pin__is_set__impl
#define set__impl K2_unused__pin__set__impl

K2_unused__pin_t const K2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K2_unused__pin__is_set__impl
#define set__impl K2_unused__pin__set__impl


;

#define is_set__impl K2_unused__pin__is_set__impl
#define set__impl K2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K2_unused__init() {
    K2_unused__ddr.set(0);
    K2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K2_unused_t;

extern K2_unused_t const K2_unused;

static AKAT_FORCE_INLINE u8 K2_unused__is_set__impl() {
#define is_set__impl K2_unused__is_set__impl
    return K2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K2_unused__is_set__impl

K2_unused_t const K2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K2_unused__is_set__impl


;

#define is_set__impl K2_unused__is_set__impl




#undef is_set__impl
;



;
; // 87   PK2 ( ADC10/PCINT18 ) Analog pin 10
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K1_unused__port_t;

extern K1_unused__port_t const K1_unused__port;

static AKAT_FORCE_INLINE void K1_unused__port__set__impl(u8 state) {
#define set__impl K1_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 1;  //Set PORTK of K1 to 1
    } else {
        PORTK &= ~(1 << 1);  //Set PORTK of K1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K1_unused__port__is_set__impl() {
#define is_set__impl K1_unused__port__is_set__impl
#define set__impl K1_unused__port__set__impl
    return PORTK & (1 << 1);  //Get value of PORTK for K1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K1_unused__port__is_set__impl
#define set__impl K1_unused__port__set__impl

K1_unused__port_t const K1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K1_unused__port__is_set__impl
#define set__impl K1_unused__port__set__impl


;

#define is_set__impl K1_unused__port__is_set__impl
#define set__impl K1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K1_unused__ddr_t;

extern K1_unused__ddr_t const K1_unused__ddr;

static AKAT_FORCE_INLINE void K1_unused__ddr__set__impl(u8 state) {
#define set__impl K1_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 1;  //Set DDRK of K1 to 1
    } else {
        DDRK &= ~(1 << 1);  //Set DDRK of K1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K1_unused__ddr__is_set__impl() {
#define is_set__impl K1_unused__ddr__is_set__impl
#define set__impl K1_unused__ddr__set__impl
    return DDRK & (1 << 1);  //Get value of DDRK for K1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K1_unused__ddr__is_set__impl
#define set__impl K1_unused__ddr__set__impl

K1_unused__ddr_t const K1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K1_unused__ddr__is_set__impl
#define set__impl K1_unused__ddr__set__impl


;

#define is_set__impl K1_unused__ddr__is_set__impl
#define set__impl K1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K1_unused__pin_t;

extern K1_unused__pin_t const K1_unused__pin;

static AKAT_FORCE_INLINE void K1_unused__pin__set__impl(u8 state) {
#define set__impl K1_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 1;  //Set PINK of K1 to 1
    } else {
        PINK &= ~(1 << 1);  //Set PINK of K1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K1_unused__pin__is_set__impl() {
#define is_set__impl K1_unused__pin__is_set__impl
#define set__impl K1_unused__pin__set__impl
    return PINK & (1 << 1);  //Get value of PINK for K1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K1_unused__pin__is_set__impl
#define set__impl K1_unused__pin__set__impl

K1_unused__pin_t const K1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K1_unused__pin__is_set__impl
#define set__impl K1_unused__pin__set__impl


;

#define is_set__impl K1_unused__pin__is_set__impl
#define set__impl K1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K1_unused__init() {
    K1_unused__ddr.set(0);
    K1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K1_unused_t;

extern K1_unused_t const K1_unused;

static AKAT_FORCE_INLINE u8 K1_unused__is_set__impl() {
#define is_set__impl K1_unused__is_set__impl
    return K1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K1_unused__is_set__impl

K1_unused_t const K1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K1_unused__is_set__impl


;

#define is_set__impl K1_unused__is_set__impl




#undef is_set__impl
;



;
; // 88   PK1 ( ADC9/PCINT17 ) Analog pin 9
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K0_unused__port_t;

extern K0_unused__port_t const K0_unused__port;

static AKAT_FORCE_INLINE void K0_unused__port__set__impl(u8 state) {
#define set__impl K0_unused__port__set__impl

    if (state) {
        PORTK |= 1 << 0;  //Set PORTK of K0 to 1
    } else {
        PORTK &= ~(1 << 0);  //Set PORTK of K0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K0_unused__port__is_set__impl() {
#define is_set__impl K0_unused__port__is_set__impl
#define set__impl K0_unused__port__set__impl
    return PORTK & (1 << 0);  //Get value of PORTK for K0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K0_unused__port__is_set__impl
#define set__impl K0_unused__port__set__impl

K0_unused__port_t const K0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl K0_unused__port__is_set__impl
#define set__impl K0_unused__port__set__impl


;

#define is_set__impl K0_unused__port__is_set__impl
#define set__impl K0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K0_unused__ddr_t;

extern K0_unused__ddr_t const K0_unused__ddr;

static AKAT_FORCE_INLINE void K0_unused__ddr__set__impl(u8 state) {
#define set__impl K0_unused__ddr__set__impl

    if (state) {
        DDRK |= 1 << 0;  //Set DDRK of K0 to 1
    } else {
        DDRK &= ~(1 << 0);  //Set DDRK of K0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K0_unused__ddr__is_set__impl() {
#define is_set__impl K0_unused__ddr__is_set__impl
#define set__impl K0_unused__ddr__set__impl
    return DDRK & (1 << 0);  //Get value of DDRK for K0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K0_unused__ddr__is_set__impl
#define set__impl K0_unused__ddr__set__impl

K0_unused__ddr_t const K0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K0_unused__ddr__is_set__impl
#define set__impl K0_unused__ddr__set__impl


;

#define is_set__impl K0_unused__ddr__is_set__impl
#define set__impl K0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} K0_unused__pin_t;

extern K0_unused__pin_t const K0_unused__pin;

static AKAT_FORCE_INLINE void K0_unused__pin__set__impl(u8 state) {
#define set__impl K0_unused__pin__set__impl

    if (state) {
        PINK |= 1 << 0;  //Set PINK of K0 to 1
    } else {
        PINK &= ~(1 << 0);  //Set PINK of K0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 K0_unused__pin__is_set__impl() {
#define is_set__impl K0_unused__pin__is_set__impl
#define set__impl K0_unused__pin__set__impl
    return PINK & (1 << 0);  //Get value of PINK for K0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl K0_unused__pin__is_set__impl
#define set__impl K0_unused__pin__set__impl

K0_unused__pin_t const K0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl K0_unused__pin__is_set__impl
#define set__impl K0_unused__pin__set__impl


;

#define is_set__impl K0_unused__pin__is_set__impl
#define set__impl K0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void K0_unused__init() {
    K0_unused__ddr.set(0);
    K0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} K0_unused_t;

extern K0_unused_t const K0_unused;

static AKAT_FORCE_INLINE u8 K0_unused__is_set__impl() {
#define is_set__impl K0_unused__is_set__impl
    return K0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl K0_unused__is_set__impl

K0_unused_t const K0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl K0_unused__is_set__impl


;

#define is_set__impl K0_unused__is_set__impl




#undef is_set__impl
;



;
; // 89   PK0 ( ADC8/PCINT16 ) Analog pin 8
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F7_unused__port_t;

extern F7_unused__port_t const F7_unused__port;

static AKAT_FORCE_INLINE void F7_unused__port__set__impl(u8 state) {
#define set__impl F7_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 7;  //Set PORTF of F7 to 1
    } else {
        PORTF &= ~(1 << 7);  //Set PORTF of F7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F7_unused__port__is_set__impl() {
#define is_set__impl F7_unused__port__is_set__impl
#define set__impl F7_unused__port__set__impl
    return PORTF & (1 << 7);  //Get value of PORTF for F7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F7_unused__port__is_set__impl
#define set__impl F7_unused__port__set__impl

F7_unused__port_t const F7_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F7_unused__port__is_set__impl
#define set__impl F7_unused__port__set__impl


;

#define is_set__impl F7_unused__port__is_set__impl
#define set__impl F7_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F7_unused__ddr_t;

extern F7_unused__ddr_t const F7_unused__ddr;

static AKAT_FORCE_INLINE void F7_unused__ddr__set__impl(u8 state) {
#define set__impl F7_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 7;  //Set DDRF of F7 to 1
    } else {
        DDRF &= ~(1 << 7);  //Set DDRF of F7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F7_unused__ddr__is_set__impl() {
#define is_set__impl F7_unused__ddr__is_set__impl
#define set__impl F7_unused__ddr__set__impl
    return DDRF & (1 << 7);  //Get value of DDRF for F7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F7_unused__ddr__is_set__impl
#define set__impl F7_unused__ddr__set__impl

F7_unused__ddr_t const F7_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F7_unused__ddr__is_set__impl
#define set__impl F7_unused__ddr__set__impl


;

#define is_set__impl F7_unused__ddr__is_set__impl
#define set__impl F7_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F7_unused__pin_t;

extern F7_unused__pin_t const F7_unused__pin;

static AKAT_FORCE_INLINE void F7_unused__pin__set__impl(u8 state) {
#define set__impl F7_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 7;  //Set PINF of F7 to 1
    } else {
        PINF &= ~(1 << 7);  //Set PINF of F7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F7_unused__pin__is_set__impl() {
#define is_set__impl F7_unused__pin__is_set__impl
#define set__impl F7_unused__pin__set__impl
    return PINF & (1 << 7);  //Get value of PINF for F7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F7_unused__pin__is_set__impl
#define set__impl F7_unused__pin__set__impl

F7_unused__pin_t const F7_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F7_unused__pin__is_set__impl
#define set__impl F7_unused__pin__set__impl


;

#define is_set__impl F7_unused__pin__is_set__impl
#define set__impl F7_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F7_unused__init() {
    F7_unused__ddr.set(0);
    F7_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F7_unused_t;

extern F7_unused_t const F7_unused;

static AKAT_FORCE_INLINE u8 F7_unused__is_set__impl() {
#define is_set__impl F7_unused__is_set__impl
    return F7_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F7_unused__is_set__impl

F7_unused_t const F7_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F7_unused__is_set__impl


;

#define is_set__impl F7_unused__is_set__impl




#undef is_set__impl
;



;
; // 90   PF7 ( ADC7/TDI ) Analog pin 7
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F6_unused__port_t;

extern F6_unused__port_t const F6_unused__port;

static AKAT_FORCE_INLINE void F6_unused__port__set__impl(u8 state) {
#define set__impl F6_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 6;  //Set PORTF of F6 to 1
    } else {
        PORTF &= ~(1 << 6);  //Set PORTF of F6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F6_unused__port__is_set__impl() {
#define is_set__impl F6_unused__port__is_set__impl
#define set__impl F6_unused__port__set__impl
    return PORTF & (1 << 6);  //Get value of PORTF for F6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F6_unused__port__is_set__impl
#define set__impl F6_unused__port__set__impl

F6_unused__port_t const F6_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F6_unused__port__is_set__impl
#define set__impl F6_unused__port__set__impl


;

#define is_set__impl F6_unused__port__is_set__impl
#define set__impl F6_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F6_unused__ddr_t;

extern F6_unused__ddr_t const F6_unused__ddr;

static AKAT_FORCE_INLINE void F6_unused__ddr__set__impl(u8 state) {
#define set__impl F6_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 6;  //Set DDRF of F6 to 1
    } else {
        DDRF &= ~(1 << 6);  //Set DDRF of F6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F6_unused__ddr__is_set__impl() {
#define is_set__impl F6_unused__ddr__is_set__impl
#define set__impl F6_unused__ddr__set__impl
    return DDRF & (1 << 6);  //Get value of DDRF for F6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F6_unused__ddr__is_set__impl
#define set__impl F6_unused__ddr__set__impl

F6_unused__ddr_t const F6_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F6_unused__ddr__is_set__impl
#define set__impl F6_unused__ddr__set__impl


;

#define is_set__impl F6_unused__ddr__is_set__impl
#define set__impl F6_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F6_unused__pin_t;

extern F6_unused__pin_t const F6_unused__pin;

static AKAT_FORCE_INLINE void F6_unused__pin__set__impl(u8 state) {
#define set__impl F6_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 6;  //Set PINF of F6 to 1
    } else {
        PINF &= ~(1 << 6);  //Set PINF of F6 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F6_unused__pin__is_set__impl() {
#define is_set__impl F6_unused__pin__is_set__impl
#define set__impl F6_unused__pin__set__impl
    return PINF & (1 << 6);  //Get value of PINF for F6
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F6_unused__pin__is_set__impl
#define set__impl F6_unused__pin__set__impl

F6_unused__pin_t const F6_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F6_unused__pin__is_set__impl
#define set__impl F6_unused__pin__set__impl


;

#define is_set__impl F6_unused__pin__is_set__impl
#define set__impl F6_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F6_unused__init() {
    F6_unused__ddr.set(0);
    F6_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F6_unused_t;

extern F6_unused_t const F6_unused;

static AKAT_FORCE_INLINE u8 F6_unused__is_set__impl() {
#define is_set__impl F6_unused__is_set__impl
    return F6_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F6_unused__is_set__impl

F6_unused_t const F6_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F6_unused__is_set__impl


;

#define is_set__impl F6_unused__is_set__impl




#undef is_set__impl
;



;
; // 91   PF6 ( ADC6/TDO ) Analog pin 6
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F5_unused__port_t;

extern F5_unused__port_t const F5_unused__port;

static AKAT_FORCE_INLINE void F5_unused__port__set__impl(u8 state) {
#define set__impl F5_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 5;  //Set PORTF of F5 to 1
    } else {
        PORTF &= ~(1 << 5);  //Set PORTF of F5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F5_unused__port__is_set__impl() {
#define is_set__impl F5_unused__port__is_set__impl
#define set__impl F5_unused__port__set__impl
    return PORTF & (1 << 5);  //Get value of PORTF for F5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F5_unused__port__is_set__impl
#define set__impl F5_unused__port__set__impl

F5_unused__port_t const F5_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F5_unused__port__is_set__impl
#define set__impl F5_unused__port__set__impl


;

#define is_set__impl F5_unused__port__is_set__impl
#define set__impl F5_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F5_unused__ddr_t;

extern F5_unused__ddr_t const F5_unused__ddr;

static AKAT_FORCE_INLINE void F5_unused__ddr__set__impl(u8 state) {
#define set__impl F5_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 5;  //Set DDRF of F5 to 1
    } else {
        DDRF &= ~(1 << 5);  //Set DDRF of F5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F5_unused__ddr__is_set__impl() {
#define is_set__impl F5_unused__ddr__is_set__impl
#define set__impl F5_unused__ddr__set__impl
    return DDRF & (1 << 5);  //Get value of DDRF for F5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F5_unused__ddr__is_set__impl
#define set__impl F5_unused__ddr__set__impl

F5_unused__ddr_t const F5_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F5_unused__ddr__is_set__impl
#define set__impl F5_unused__ddr__set__impl


;

#define is_set__impl F5_unused__ddr__is_set__impl
#define set__impl F5_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F5_unused__pin_t;

extern F5_unused__pin_t const F5_unused__pin;

static AKAT_FORCE_INLINE void F5_unused__pin__set__impl(u8 state) {
#define set__impl F5_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 5;  //Set PINF of F5 to 1
    } else {
        PINF &= ~(1 << 5);  //Set PINF of F5 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F5_unused__pin__is_set__impl() {
#define is_set__impl F5_unused__pin__is_set__impl
#define set__impl F5_unused__pin__set__impl
    return PINF & (1 << 5);  //Get value of PINF for F5
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F5_unused__pin__is_set__impl
#define set__impl F5_unused__pin__set__impl

F5_unused__pin_t const F5_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F5_unused__pin__is_set__impl
#define set__impl F5_unused__pin__set__impl


;

#define is_set__impl F5_unused__pin__is_set__impl
#define set__impl F5_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F5_unused__init() {
    F5_unused__ddr.set(0);
    F5_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F5_unused_t;

extern F5_unused_t const F5_unused;

static AKAT_FORCE_INLINE u8 F5_unused__is_set__impl() {
#define is_set__impl F5_unused__is_set__impl
    return F5_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F5_unused__is_set__impl

F5_unused_t const F5_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F5_unused__is_set__impl


;

#define is_set__impl F5_unused__is_set__impl




#undef is_set__impl
;



;
; // 92   PF5 ( ADC5/TMS ) Analog pin 5
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F4_unused__port_t;

extern F4_unused__port_t const F4_unused__port;

static AKAT_FORCE_INLINE void F4_unused__port__set__impl(u8 state) {
#define set__impl F4_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 4;  //Set PORTF of F4 to 1
    } else {
        PORTF &= ~(1 << 4);  //Set PORTF of F4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F4_unused__port__is_set__impl() {
#define is_set__impl F4_unused__port__is_set__impl
#define set__impl F4_unused__port__set__impl
    return PORTF & (1 << 4);  //Get value of PORTF for F4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F4_unused__port__is_set__impl
#define set__impl F4_unused__port__set__impl

F4_unused__port_t const F4_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F4_unused__port__is_set__impl
#define set__impl F4_unused__port__set__impl


;

#define is_set__impl F4_unused__port__is_set__impl
#define set__impl F4_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F4_unused__ddr_t;

extern F4_unused__ddr_t const F4_unused__ddr;

static AKAT_FORCE_INLINE void F4_unused__ddr__set__impl(u8 state) {
#define set__impl F4_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 4;  //Set DDRF of F4 to 1
    } else {
        DDRF &= ~(1 << 4);  //Set DDRF of F4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F4_unused__ddr__is_set__impl() {
#define is_set__impl F4_unused__ddr__is_set__impl
#define set__impl F4_unused__ddr__set__impl
    return DDRF & (1 << 4);  //Get value of DDRF for F4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F4_unused__ddr__is_set__impl
#define set__impl F4_unused__ddr__set__impl

F4_unused__ddr_t const F4_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F4_unused__ddr__is_set__impl
#define set__impl F4_unused__ddr__set__impl


;

#define is_set__impl F4_unused__ddr__is_set__impl
#define set__impl F4_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F4_unused__pin_t;

extern F4_unused__pin_t const F4_unused__pin;

static AKAT_FORCE_INLINE void F4_unused__pin__set__impl(u8 state) {
#define set__impl F4_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 4;  //Set PINF of F4 to 1
    } else {
        PINF &= ~(1 << 4);  //Set PINF of F4 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F4_unused__pin__is_set__impl() {
#define is_set__impl F4_unused__pin__is_set__impl
#define set__impl F4_unused__pin__set__impl
    return PINF & (1 << 4);  //Get value of PINF for F4
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F4_unused__pin__is_set__impl
#define set__impl F4_unused__pin__set__impl

F4_unused__pin_t const F4_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F4_unused__pin__is_set__impl
#define set__impl F4_unused__pin__set__impl


;

#define is_set__impl F4_unused__pin__is_set__impl
#define set__impl F4_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F4_unused__init() {
    F4_unused__ddr.set(0);
    F4_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F4_unused_t;

extern F4_unused_t const F4_unused;

static AKAT_FORCE_INLINE u8 F4_unused__is_set__impl() {
#define is_set__impl F4_unused__is_set__impl
    return F4_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F4_unused__is_set__impl

F4_unused_t const F4_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F4_unused__is_set__impl


;

#define is_set__impl F4_unused__is_set__impl




#undef is_set__impl
;



;
; // 93   PF4 ( ADC4/TCK ) Analog pin 4
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F3_unused__port_t;

extern F3_unused__port_t const F3_unused__port;

static AKAT_FORCE_INLINE void F3_unused__port__set__impl(u8 state) {
#define set__impl F3_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 3;  //Set PORTF of F3 to 1
    } else {
        PORTF &= ~(1 << 3);  //Set PORTF of F3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F3_unused__port__is_set__impl() {
#define is_set__impl F3_unused__port__is_set__impl
#define set__impl F3_unused__port__set__impl
    return PORTF & (1 << 3);  //Get value of PORTF for F3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F3_unused__port__is_set__impl
#define set__impl F3_unused__port__set__impl

F3_unused__port_t const F3_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F3_unused__port__is_set__impl
#define set__impl F3_unused__port__set__impl


;

#define is_set__impl F3_unused__port__is_set__impl
#define set__impl F3_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F3_unused__ddr_t;

extern F3_unused__ddr_t const F3_unused__ddr;

static AKAT_FORCE_INLINE void F3_unused__ddr__set__impl(u8 state) {
#define set__impl F3_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 3;  //Set DDRF of F3 to 1
    } else {
        DDRF &= ~(1 << 3);  //Set DDRF of F3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F3_unused__ddr__is_set__impl() {
#define is_set__impl F3_unused__ddr__is_set__impl
#define set__impl F3_unused__ddr__set__impl
    return DDRF & (1 << 3);  //Get value of DDRF for F3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F3_unused__ddr__is_set__impl
#define set__impl F3_unused__ddr__set__impl

F3_unused__ddr_t const F3_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F3_unused__ddr__is_set__impl
#define set__impl F3_unused__ddr__set__impl


;

#define is_set__impl F3_unused__ddr__is_set__impl
#define set__impl F3_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F3_unused__pin_t;

extern F3_unused__pin_t const F3_unused__pin;

static AKAT_FORCE_INLINE void F3_unused__pin__set__impl(u8 state) {
#define set__impl F3_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 3;  //Set PINF of F3 to 1
    } else {
        PINF &= ~(1 << 3);  //Set PINF of F3 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F3_unused__pin__is_set__impl() {
#define is_set__impl F3_unused__pin__is_set__impl
#define set__impl F3_unused__pin__set__impl
    return PINF & (1 << 3);  //Get value of PINF for F3
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F3_unused__pin__is_set__impl
#define set__impl F3_unused__pin__set__impl

F3_unused__pin_t const F3_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F3_unused__pin__is_set__impl
#define set__impl F3_unused__pin__set__impl


;

#define is_set__impl F3_unused__pin__is_set__impl
#define set__impl F3_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F3_unused__init() {
    F3_unused__ddr.set(0);
    F3_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F3_unused_t;

extern F3_unused_t const F3_unused;

static AKAT_FORCE_INLINE u8 F3_unused__is_set__impl() {
#define is_set__impl F3_unused__is_set__impl
    return F3_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F3_unused__is_set__impl

F3_unused_t const F3_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F3_unused__is_set__impl


;

#define is_set__impl F3_unused__is_set__impl




#undef is_set__impl
;



;
; // 94   PF3 ( ADC3 ) Analog pin 3
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F2_unused__port_t;

extern F2_unused__port_t const F2_unused__port;

static AKAT_FORCE_INLINE void F2_unused__port__set__impl(u8 state) {
#define set__impl F2_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 2;  //Set PORTF of F2 to 1
    } else {
        PORTF &= ~(1 << 2);  //Set PORTF of F2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F2_unused__port__is_set__impl() {
#define is_set__impl F2_unused__port__is_set__impl
#define set__impl F2_unused__port__set__impl
    return PORTF & (1 << 2);  //Get value of PORTF for F2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F2_unused__port__is_set__impl
#define set__impl F2_unused__port__set__impl

F2_unused__port_t const F2_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F2_unused__port__is_set__impl
#define set__impl F2_unused__port__set__impl


;

#define is_set__impl F2_unused__port__is_set__impl
#define set__impl F2_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F2_unused__ddr_t;

extern F2_unused__ddr_t const F2_unused__ddr;

static AKAT_FORCE_INLINE void F2_unused__ddr__set__impl(u8 state) {
#define set__impl F2_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 2;  //Set DDRF of F2 to 1
    } else {
        DDRF &= ~(1 << 2);  //Set DDRF of F2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F2_unused__ddr__is_set__impl() {
#define is_set__impl F2_unused__ddr__is_set__impl
#define set__impl F2_unused__ddr__set__impl
    return DDRF & (1 << 2);  //Get value of DDRF for F2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F2_unused__ddr__is_set__impl
#define set__impl F2_unused__ddr__set__impl

F2_unused__ddr_t const F2_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F2_unused__ddr__is_set__impl
#define set__impl F2_unused__ddr__set__impl


;

#define is_set__impl F2_unused__ddr__is_set__impl
#define set__impl F2_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F2_unused__pin_t;

extern F2_unused__pin_t const F2_unused__pin;

static AKAT_FORCE_INLINE void F2_unused__pin__set__impl(u8 state) {
#define set__impl F2_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 2;  //Set PINF of F2 to 1
    } else {
        PINF &= ~(1 << 2);  //Set PINF of F2 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F2_unused__pin__is_set__impl() {
#define is_set__impl F2_unused__pin__is_set__impl
#define set__impl F2_unused__pin__set__impl
    return PINF & (1 << 2);  //Get value of PINF for F2
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F2_unused__pin__is_set__impl
#define set__impl F2_unused__pin__set__impl

F2_unused__pin_t const F2_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F2_unused__pin__is_set__impl
#define set__impl F2_unused__pin__set__impl


;

#define is_set__impl F2_unused__pin__is_set__impl
#define set__impl F2_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F2_unused__init() {
    F2_unused__ddr.set(0);
    F2_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F2_unused_t;

extern F2_unused_t const F2_unused;

static AKAT_FORCE_INLINE u8 F2_unused__is_set__impl() {
#define is_set__impl F2_unused__is_set__impl
    return F2_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F2_unused__is_set__impl

F2_unused_t const F2_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F2_unused__is_set__impl


;

#define is_set__impl F2_unused__is_set__impl




#undef is_set__impl
;



;
; // 95   PF2 ( ADC2 ) Analog pin 2
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F1_unused__port_t;

extern F1_unused__port_t const F1_unused__port;

static AKAT_FORCE_INLINE void F1_unused__port__set__impl(u8 state) {
#define set__impl F1_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 1;  //Set PORTF of F1 to 1
    } else {
        PORTF &= ~(1 << 1);  //Set PORTF of F1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F1_unused__port__is_set__impl() {
#define is_set__impl F1_unused__port__is_set__impl
#define set__impl F1_unused__port__set__impl
    return PORTF & (1 << 1);  //Get value of PORTF for F1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F1_unused__port__is_set__impl
#define set__impl F1_unused__port__set__impl

F1_unused__port_t const F1_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F1_unused__port__is_set__impl
#define set__impl F1_unused__port__set__impl


;

#define is_set__impl F1_unused__port__is_set__impl
#define set__impl F1_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F1_unused__ddr_t;

extern F1_unused__ddr_t const F1_unused__ddr;

static AKAT_FORCE_INLINE void F1_unused__ddr__set__impl(u8 state) {
#define set__impl F1_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 1;  //Set DDRF of F1 to 1
    } else {
        DDRF &= ~(1 << 1);  //Set DDRF of F1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F1_unused__ddr__is_set__impl() {
#define is_set__impl F1_unused__ddr__is_set__impl
#define set__impl F1_unused__ddr__set__impl
    return DDRF & (1 << 1);  //Get value of DDRF for F1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F1_unused__ddr__is_set__impl
#define set__impl F1_unused__ddr__set__impl

F1_unused__ddr_t const F1_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F1_unused__ddr__is_set__impl
#define set__impl F1_unused__ddr__set__impl


;

#define is_set__impl F1_unused__ddr__is_set__impl
#define set__impl F1_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F1_unused__pin_t;

extern F1_unused__pin_t const F1_unused__pin;

static AKAT_FORCE_INLINE void F1_unused__pin__set__impl(u8 state) {
#define set__impl F1_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 1;  //Set PINF of F1 to 1
    } else {
        PINF &= ~(1 << 1);  //Set PINF of F1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F1_unused__pin__is_set__impl() {
#define is_set__impl F1_unused__pin__is_set__impl
#define set__impl F1_unused__pin__set__impl
    return PINF & (1 << 1);  //Get value of PINF for F1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F1_unused__pin__is_set__impl
#define set__impl F1_unused__pin__set__impl

F1_unused__pin_t const F1_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F1_unused__pin__is_set__impl
#define set__impl F1_unused__pin__set__impl


;

#define is_set__impl F1_unused__pin__is_set__impl
#define set__impl F1_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F1_unused__init() {
    F1_unused__ddr.set(0);
    F1_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F1_unused_t;

extern F1_unused_t const F1_unused;

static AKAT_FORCE_INLINE u8 F1_unused__is_set__impl() {
#define is_set__impl F1_unused__is_set__impl
    return F1_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F1_unused__is_set__impl

F1_unused_t const F1_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F1_unused__is_set__impl


;

#define is_set__impl F1_unused__is_set__impl




#undef is_set__impl
;



;
; // 96   PF1 ( ADC1 ) Analog pin 1
typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F0_unused__port_t;

extern F0_unused__port_t const F0_unused__port;

static AKAT_FORCE_INLINE void F0_unused__port__set__impl(u8 state) {
#define set__impl F0_unused__port__set__impl

    if (state) {
        PORTF |= 1 << 0;  //Set PORTF of F0 to 1
    } else {
        PORTF &= ~(1 << 0);  //Set PORTF of F0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F0_unused__port__is_set__impl() {
#define is_set__impl F0_unused__port__is_set__impl
#define set__impl F0_unused__port__set__impl
    return PORTF & (1 << 0);  //Get value of PORTF for F0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F0_unused__port__is_set__impl
#define set__impl F0_unused__port__set__impl

F0_unused__port_t const F0_unused__port = {.set = &set__impl
                                           ,
                                           .is_set = &is_set__impl
                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl F0_unused__port__is_set__impl
#define set__impl F0_unused__port__set__impl


;

#define is_set__impl F0_unused__port__is_set__impl
#define set__impl F0_unused__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F0_unused__ddr_t;

extern F0_unused__ddr_t const F0_unused__ddr;

static AKAT_FORCE_INLINE void F0_unused__ddr__set__impl(u8 state) {
#define set__impl F0_unused__ddr__set__impl

    if (state) {
        DDRF |= 1 << 0;  //Set DDRF of F0 to 1
    } else {
        DDRF &= ~(1 << 0);  //Set DDRF of F0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F0_unused__ddr__is_set__impl() {
#define is_set__impl F0_unused__ddr__is_set__impl
#define set__impl F0_unused__ddr__set__impl
    return DDRF & (1 << 0);  //Get value of DDRF for F0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F0_unused__ddr__is_set__impl
#define set__impl F0_unused__ddr__set__impl

F0_unused__ddr_t const F0_unused__ddr = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F0_unused__ddr__is_set__impl
#define set__impl F0_unused__ddr__set__impl


;

#define is_set__impl F0_unused__ddr__is_set__impl
#define set__impl F0_unused__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} F0_unused__pin_t;

extern F0_unused__pin_t const F0_unused__pin;

static AKAT_FORCE_INLINE void F0_unused__pin__set__impl(u8 state) {
#define set__impl F0_unused__pin__set__impl

    if (state) {
        PINF |= 1 << 0;  //Set PINF of F0 to 1
    } else {
        PINF &= ~(1 << 0);  //Set PINF of F0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 F0_unused__pin__is_set__impl() {
#define is_set__impl F0_unused__pin__is_set__impl
#define set__impl F0_unused__pin__set__impl
    return PINF & (1 << 0);  //Get value of PINF for F0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl F0_unused__pin__is_set__impl
#define set__impl F0_unused__pin__set__impl

F0_unused__pin_t const F0_unused__pin = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl F0_unused__pin__is_set__impl
#define set__impl F0_unused__pin__set__impl


;

#define is_set__impl F0_unused__pin__is_set__impl
#define set__impl F0_unused__pin__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void F0_unused__init() {
    F0_unused__ddr.set(0);
    F0_unused__port.set(1);
}

;





typedef struct {
    u8 (* const is_set)();
} F0_unused_t;

extern F0_unused_t const F0_unused;

static AKAT_FORCE_INLINE u8 F0_unused__is_set__impl() {
#define is_set__impl F0_unused__is_set__impl
    return F0_unused__pin.is_set();
#undef is_set__impl
}
#define is_set__impl F0_unused__is_set__impl

F0_unused_t const F0_unused = {.is_set = &is_set__impl
                              };


#undef is_set__impl
#define is_set__impl F0_unused__is_set__impl


;

#define is_set__impl F0_unused__is_set__impl




#undef is_set__impl
;



;
; // 97   PF0 ( ADC0 ) Analog pin 0
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

static volatile u8 debug_bytes_buf[AK_DEBUG_BUF_SIZE] = {};
static volatile u8 debug_next_empty_idx = 0;
static volatile u8 debug_next_read_idx = 0;
static volatile u8 debug_overflow_count = 0;

//Make volatile so we can use it from ISR (in theory)
;
;
;
;
;


static AKAT_UNUSED void add_debug_byte(const u8 b) {
    u8 new_next_empty_idx = (debug_next_empty_idx + AKAT_ONE) & (AK_DEBUG_BUF_SIZE - 1);

    if (new_next_empty_idx == debug_next_read_idx) {
        debug_overflow_count += AKAT_ONE;

        //Don't let it overflow!
        if (!debug_overflow_count) {
            debug_overflow_count -= AKAT_ONE;
        }
    } else {
        debug_bytes_buf[debug_next_empty_idx] = b;
        debug_next_empty_idx = new_next_empty_idx;
    }
}

;




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Uptime

static u32 uptime_deciseconds = 0;

;
;



static AKAT_FORCE_INLINE void akat_on_every_decisecond();

// Can't use LOW register here!
/* Using register r16 for akat_every_decisecond_run_required */;

register u8 akat_every_decisecond_run_required asm ("r16");

;
;


static AKAT_FORCE_INLINE void akat_on_every_decisecond_runner() {
//Tell gcc that this variable can be changed somehow (in our case via ISR)
    AKAT_FLUSH_REG_VAR(akat_every_decisecond_run_required);

    if (akat_every_decisecond_run_required) {
        akat_every_decisecond_run_required = AKAT_FALSE;
        akat_on_every_decisecond();
    }
}

;





ISR(TIMER1_COMPA_vect, ISR_NAKED) {
    // NOTE: Make sure that 'akat_every_decisecond_run_required' is not a register under R16!
    // NOTE: Otherwise we have to save SREG. That's why we use assembler directly here.
    asm volatile("ldi %0, 0x01" : "=r" (akat_every_decisecond_run_required));
    asm volatile("reti");
}


static AKAT_FORCE_INLINE void uptime_ticker() {
    uptime_deciseconds += 1;
}

;





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Led pins

typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} blue_led__port_t;

extern blue_led__port_t const blue_led__port;

static AKAT_FORCE_INLINE void blue_led__port__set__impl(u8 state) {
#define set__impl blue_led__port__set__impl

    if (state) {
        PORTB |= 1 << 7;  //Set PORTB of B7 to 1
    } else {
        PORTB &= ~(1 << 7);  //Set PORTB of B7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 blue_led__port__is_set__impl() {
#define is_set__impl blue_led__port__is_set__impl
#define set__impl blue_led__port__set__impl
    return PORTB & (1 << 7);  //Get value of PORTB for B7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl blue_led__port__is_set__impl
#define set__impl blue_led__port__set__impl

blue_led__port_t const blue_led__port = {.set = &set__impl
                                         ,
                                         .is_set = &is_set__impl
                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl blue_led__port__is_set__impl
#define set__impl blue_led__port__set__impl


;

#define is_set__impl blue_led__port__is_set__impl
#define set__impl blue_led__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} blue_led__ddr_t;

extern blue_led__ddr_t const blue_led__ddr;

static AKAT_FORCE_INLINE void blue_led__ddr__set__impl(u8 state) {
#define set__impl blue_led__ddr__set__impl

    if (state) {
        DDRB |= 1 << 7;  //Set DDRB of B7 to 1
    } else {
        DDRB &= ~(1 << 7);  //Set DDRB of B7 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 blue_led__ddr__is_set__impl() {
#define is_set__impl blue_led__ddr__is_set__impl
#define set__impl blue_led__ddr__set__impl
    return DDRB & (1 << 7);  //Get value of DDRB for B7
#undef is_set__impl
#undef set__impl
}
#define is_set__impl blue_led__ddr__is_set__impl
#define set__impl blue_led__ddr__set__impl

blue_led__ddr_t const blue_led__ddr = {.set = &set__impl
                                       ,
                                       .is_set = &is_set__impl
                                      };


#undef is_set__impl
#undef set__impl
#define is_set__impl blue_led__ddr__is_set__impl
#define set__impl blue_led__ddr__set__impl


;

#define is_set__impl blue_led__ddr__is_set__impl
#define set__impl blue_led__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



static AKAT_FORCE_INLINE void blue_led__init() {
    blue_led__ddr.set(1); //Init B7 as output
}

;





typedef struct {
    void (* const set)(u8 state);
} blue_led_t;

extern blue_led_t const blue_led;

static AKAT_FORCE_INLINE void blue_led__set__impl(u8 state) {
#define set__impl blue_led__set__impl
    blue_led__port.set(state);
#undef set__impl
}
#define set__impl blue_led__set__impl

blue_led_t const blue_led = {.set = &set__impl
                            };


#undef set__impl
#define set__impl blue_led__set__impl


;

#define set__impl blue_led__set__impl




#undef set__impl
;



;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Watchdog

#include <avr/wdt.h>

static AKAT_FORCE_INLINE void watchdog_init() {
    wdt_enable(WDTO_8S);
}

;





static AKAT_FORCE_INLINE void watchdog_reset() {
    wdt_reset();
}

;




;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Activity indication

// This piece of code will be executed in akat event/thread loop every 1/10 second.
// We use to turn the blue led ON and OFF

static u8 activity_led__state = 0;
static AKAT_FORCE_INLINE void activity_led() {
#define state activity_led__state
    ;
    blue_led.set(state);
    state = !state;
#undef state
}

;






////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// DS18B20

// Read temperature from DS18B20.
// DS18B20 is supposed to be connected to the given port and must be properly powered.
// (parasitic powering mode is not supported/tested).
// DS18B20 must be the only device connected to the pin because we use SKIP-ROM command.

static void ds18b20_ticker();

typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_aqua__pin__port_t;

extern ds18b20_aqua__pin__port_t const ds18b20_aqua__pin__port;

static AKAT_FORCE_INLINE void ds18b20_aqua__pin__port__set__impl(u8 state) {
#define set__impl ds18b20_aqua__pin__port__set__impl

    if (state) {
        PORTA |= 1 << 0;  //Set PORTA of A0 to 1
    } else {
        PORTA &= ~(1 << 0);  //Set PORTA of A0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__pin__port__is_set__impl() {
#define is_set__impl ds18b20_aqua__pin__port__is_set__impl
#define set__impl ds18b20_aqua__pin__port__set__impl
    return PORTA & (1 << 0);  //Get value of PORTA for A0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_aqua__pin__port__is_set__impl
#define set__impl ds18b20_aqua__pin__port__set__impl

ds18b20_aqua__pin__port_t const ds18b20_aqua__pin__port = {.set = &set__impl
                                                           ,
                                                           .is_set = &is_set__impl
                                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_aqua__pin__port__is_set__impl
#define set__impl ds18b20_aqua__pin__port__set__impl


;

#define is_set__impl ds18b20_aqua__pin__port__is_set__impl
#define set__impl ds18b20_aqua__pin__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_aqua__pin__ddr_t;

extern ds18b20_aqua__pin__ddr_t const ds18b20_aqua__pin__ddr;

static AKAT_FORCE_INLINE void ds18b20_aqua__pin__ddr__set__impl(u8 state) {
#define set__impl ds18b20_aqua__pin__ddr__set__impl

    if (state) {
        DDRA |= 1 << 0;  //Set DDRA of A0 to 1
    } else {
        DDRA &= ~(1 << 0);  //Set DDRA of A0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__pin__ddr__is_set__impl() {
#define is_set__impl ds18b20_aqua__pin__ddr__is_set__impl
#define set__impl ds18b20_aqua__pin__ddr__set__impl
    return DDRA & (1 << 0);  //Get value of DDRA for A0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_aqua__pin__ddr__is_set__impl
#define set__impl ds18b20_aqua__pin__ddr__set__impl

ds18b20_aqua__pin__ddr_t const ds18b20_aqua__pin__ddr = {.set = &set__impl
                                                         ,
                                                         .is_set = &is_set__impl
                                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_aqua__pin__ddr__is_set__impl
#define set__impl ds18b20_aqua__pin__ddr__set__impl


;

#define is_set__impl ds18b20_aqua__pin__ddr__is_set__impl
#define set__impl ds18b20_aqua__pin__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_aqua__pin__pin_t;

extern ds18b20_aqua__pin__pin_t const ds18b20_aqua__pin__pin;

static AKAT_FORCE_INLINE void ds18b20_aqua__pin__pin__set__impl(u8 state) {
#define set__impl ds18b20_aqua__pin__pin__set__impl

    if (state) {
        PINA |= 1 << 0;  //Set PINA of A0 to 1
    } else {
        PINA &= ~(1 << 0);  //Set PINA of A0 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__pin__pin__is_set__impl() {
#define is_set__impl ds18b20_aqua__pin__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__pin__set__impl
    return PINA & (1 << 0);  //Get value of PINA for A0
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_aqua__pin__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__pin__set__impl

ds18b20_aqua__pin__pin_t const ds18b20_aqua__pin__pin = {.set = &set__impl
                                                         ,
                                                         .is_set = &is_set__impl
                                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_aqua__pin__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__pin__set__impl


;

#define is_set__impl ds18b20_aqua__pin__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__pin__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set_input_mode)();
    void (* const set_output_mode)();
    u8 (* const is_set)();
    void (* const set)(u8 state);
} ds18b20_aqua__pin_t;

extern ds18b20_aqua__pin_t const ds18b20_aqua__pin;

static AKAT_FORCE_INLINE void ds18b20_aqua__pin__set_input_mode__impl() {
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
    ds18b20_aqua__pin__ddr.set(0);
    ds18b20_aqua__pin__port.set(1);
#undef set_input_mode__impl
}
static AKAT_FORCE_INLINE void ds18b20_aqua__pin__set_output_mode__impl() {
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl
    ds18b20_aqua__pin__ddr.set(1);
#undef set_input_mode__impl
#undef set_output_mode__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__pin__is_set__impl() {
#define is_set__impl ds18b20_aqua__pin__is_set__impl
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl
    return ds18b20_aqua__pin__pin.is_set();
#undef is_set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
}
static AKAT_FORCE_INLINE void ds18b20_aqua__pin__set__impl(u8 state) {
#define is_set__impl ds18b20_aqua__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__set__impl
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl
    ds18b20_aqua__pin__port.set(state);
#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
}
#define is_set__impl ds18b20_aqua__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__set__impl
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl

ds18b20_aqua__pin_t const ds18b20_aqua__pin = {.set_input_mode = &set_input_mode__impl
                                               ,
                                               .set_output_mode = &set_output_mode__impl
                                                       ,
                                               .is_set = &is_set__impl
                                                       ,
                                               .set = &set__impl
                                              };


#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
#define is_set__impl ds18b20_aqua__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__set__impl
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl


;

#define is_set__impl ds18b20_aqua__pin__is_set__impl
#define set__impl ds18b20_aqua__pin__set__impl
#define set_input_mode__impl ds18b20_aqua__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_aqua__pin__set_output_mode__impl







#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
;



;

static AKAT_FORCE_INLINE void ds18b20_aqua__init() {
//Safe state - input
    ds18b20_aqua__pin.set_input_mode();
}

;





// Static variable for communication between our thread and other parts of code
static u8 ds18b20_aqua__connected = 0;
static u8 ds18b20_aqua__received_byte = 0;
static u8 ds18b20_aqua__scratchpad[9] = {};
static u8 ds18b20_aqua__updated_deciseconds_ago = 255;
static u8 ds18b20_aqua__crc_errors = 0;
static u8 ds18b20_aqua__disconnects = 0;
static u16 ds18b20_aqua__temperatureX16 = 0;

//Whether reset procedure detect presence pulse or not
;

//Last received byte
;

//Last scratchpad
;

//Statistics
;
;
;

//Temperature (must be divided by 16 to convert to degrees)
;
;



static AKAT_FORCE_INLINE void ds18b20_aqua__ticker() {
//Maintain freshness
    ds18b20_aqua__updated_deciseconds_ago += AKAT_ONE;

    if (!ds18b20_aqua__updated_deciseconds_ago) {//We can't go beyond 255
        ds18b20_aqua__updated_deciseconds_ago -= AKAT_ONE;
    }
}

;





typedef struct {
    u8 (* const get_updated_deciseconds_ago)();
    u8 (* const get_disconnects)();
    u8 (* const get_crc_errors)();
    u16 (* const get_temperatureX16)();
} ds18b20_aqua_t;

extern ds18b20_aqua_t const ds18b20_aqua;

static AKAT_FORCE_INLINE u8 ds18b20_aqua__get_updated_deciseconds_ago__impl() {
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl
    return ds18b20_aqua__updated_deciseconds_ago;
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__get_disconnects__impl() {
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl
    return ds18b20_aqua__disconnects;
#undef get_disconnects__impl
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_aqua__get_crc_errors__impl() {
#define get_crc_errors__impl ds18b20_aqua__get_crc_errors__impl
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl
    return ds18b20_aqua__crc_errors;
#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u16 ds18b20_aqua__get_temperatureX16__impl() {
#define get_crc_errors__impl ds18b20_aqua__get_crc_errors__impl
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_aqua__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl
    return ds18b20_aqua__temperatureX16;
#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
}
#define get_crc_errors__impl ds18b20_aqua__get_crc_errors__impl
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_aqua__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl

ds18b20_aqua_t const ds18b20_aqua = {.get_updated_deciseconds_ago = &get_updated_deciseconds_ago__impl
                                     ,
                                     .get_disconnects = &get_disconnects__impl
                                             ,
                                     .get_crc_errors = &get_crc_errors__impl
                                             ,
                                     .get_temperatureX16 = &get_temperatureX16__impl
                                    };


#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
#define get_crc_errors__impl ds18b20_aqua__get_crc_errors__impl
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_aqua__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl


;

#define get_crc_errors__impl ds18b20_aqua__get_crc_errors__impl
#define get_disconnects__impl ds18b20_aqua__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_aqua__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_aqua__get_updated_deciseconds_ago__impl







#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
;



;
// Read temperature from DS18B20.
// DS18B20 is supposed to be connected to the given port and must be properly powered.
// (parasitic powering mode is not supported/tested).
// DS18B20 must be the only device connected to the pin because we use SKIP-ROM command.


typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_case__pin__port_t;

extern ds18b20_case__pin__port_t const ds18b20_case__pin__port;

static AKAT_FORCE_INLINE void ds18b20_case__pin__port__set__impl(u8 state) {
#define set__impl ds18b20_case__pin__port__set__impl

    if (state) {
        PORTA |= 1 << 1;  //Set PORTA of A1 to 1
    } else {
        PORTA &= ~(1 << 1);  //Set PORTA of A1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__pin__port__is_set__impl() {
#define is_set__impl ds18b20_case__pin__port__is_set__impl
#define set__impl ds18b20_case__pin__port__set__impl
    return PORTA & (1 << 1);  //Get value of PORTA for A1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_case__pin__port__is_set__impl
#define set__impl ds18b20_case__pin__port__set__impl

ds18b20_case__pin__port_t const ds18b20_case__pin__port = {.set = &set__impl
                                                           ,
                                                           .is_set = &is_set__impl
                                                          };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_case__pin__port__is_set__impl
#define set__impl ds18b20_case__pin__port__set__impl


;

#define is_set__impl ds18b20_case__pin__port__is_set__impl
#define set__impl ds18b20_case__pin__port__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_case__pin__ddr_t;

extern ds18b20_case__pin__ddr_t const ds18b20_case__pin__ddr;

static AKAT_FORCE_INLINE void ds18b20_case__pin__ddr__set__impl(u8 state) {
#define set__impl ds18b20_case__pin__ddr__set__impl

    if (state) {
        DDRA |= 1 << 1;  //Set DDRA of A1 to 1
    } else {
        DDRA &= ~(1 << 1);  //Set DDRA of A1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__pin__ddr__is_set__impl() {
#define is_set__impl ds18b20_case__pin__ddr__is_set__impl
#define set__impl ds18b20_case__pin__ddr__set__impl
    return DDRA & (1 << 1);  //Get value of DDRA for A1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_case__pin__ddr__is_set__impl
#define set__impl ds18b20_case__pin__ddr__set__impl

ds18b20_case__pin__ddr_t const ds18b20_case__pin__ddr = {.set = &set__impl
                                                         ,
                                                         .is_set = &is_set__impl
                                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_case__pin__ddr__is_set__impl
#define set__impl ds18b20_case__pin__ddr__set__impl


;

#define is_set__impl ds18b20_case__pin__ddr__is_set__impl
#define set__impl ds18b20_case__pin__ddr__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set)(u8 state);
    u8 (* const is_set)();
} ds18b20_case__pin__pin_t;

extern ds18b20_case__pin__pin_t const ds18b20_case__pin__pin;

static AKAT_FORCE_INLINE void ds18b20_case__pin__pin__set__impl(u8 state) {
#define set__impl ds18b20_case__pin__pin__set__impl

    if (state) {
        PINA |= 1 << 1;  //Set PINA of A1 to 1
    } else {
        PINA &= ~(1 << 1);  //Set PINA of A1 to 0
    }

#undef set__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__pin__pin__is_set__impl() {
#define is_set__impl ds18b20_case__pin__pin__is_set__impl
#define set__impl ds18b20_case__pin__pin__set__impl
    return PINA & (1 << 1);  //Get value of PINA for A1
#undef is_set__impl
#undef set__impl
}
#define is_set__impl ds18b20_case__pin__pin__is_set__impl
#define set__impl ds18b20_case__pin__pin__set__impl

ds18b20_case__pin__pin_t const ds18b20_case__pin__pin = {.set = &set__impl
                                                         ,
                                                         .is_set = &is_set__impl
                                                        };


#undef is_set__impl
#undef set__impl
#define is_set__impl ds18b20_case__pin__pin__is_set__impl
#define set__impl ds18b20_case__pin__pin__set__impl


;

#define is_set__impl ds18b20_case__pin__pin__is_set__impl
#define set__impl ds18b20_case__pin__pin__set__impl





#undef is_set__impl
#undef set__impl
;



typedef struct {
    void (* const set_input_mode)();
    void (* const set_output_mode)();
    u8 (* const is_set)();
    void (* const set)(u8 state);
} ds18b20_case__pin_t;

extern ds18b20_case__pin_t const ds18b20_case__pin;

static AKAT_FORCE_INLINE void ds18b20_case__pin__set_input_mode__impl() {
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
    ds18b20_case__pin__ddr.set(0);
    ds18b20_case__pin__port.set(1);
#undef set_input_mode__impl
}
static AKAT_FORCE_INLINE void ds18b20_case__pin__set_output_mode__impl() {
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl
    ds18b20_case__pin__ddr.set(1);
#undef set_input_mode__impl
#undef set_output_mode__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__pin__is_set__impl() {
#define is_set__impl ds18b20_case__pin__is_set__impl
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl
    return ds18b20_case__pin__pin.is_set();
#undef is_set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
}
static AKAT_FORCE_INLINE void ds18b20_case__pin__set__impl(u8 state) {
#define is_set__impl ds18b20_case__pin__is_set__impl
#define set__impl ds18b20_case__pin__set__impl
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl
    ds18b20_case__pin__port.set(state);
#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
}
#define is_set__impl ds18b20_case__pin__is_set__impl
#define set__impl ds18b20_case__pin__set__impl
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl

ds18b20_case__pin_t const ds18b20_case__pin = {.set_input_mode = &set_input_mode__impl
                                               ,
                                               .set_output_mode = &set_output_mode__impl
                                                       ,
                                               .is_set = &is_set__impl
                                                       ,
                                               .set = &set__impl
                                              };


#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
#define is_set__impl ds18b20_case__pin__is_set__impl
#define set__impl ds18b20_case__pin__set__impl
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl


;

#define is_set__impl ds18b20_case__pin__is_set__impl
#define set__impl ds18b20_case__pin__set__impl
#define set_input_mode__impl ds18b20_case__pin__set_input_mode__impl
#define set_output_mode__impl ds18b20_case__pin__set_output_mode__impl







#undef is_set__impl
#undef set__impl
#undef set_input_mode__impl
#undef set_output_mode__impl
;



;

static AKAT_FORCE_INLINE void ds18b20_case__init() {
//Safe state - input
    ds18b20_case__pin.set_input_mode();
}

;





// Static variable for communication between our thread and other parts of code
static u8 ds18b20_case__connected = 0;
static u8 ds18b20_case__received_byte = 0;
static u8 ds18b20_case__scratchpad[9] = {};
static u8 ds18b20_case__updated_deciseconds_ago = 255;
static u8 ds18b20_case__crc_errors = 0;
static u8 ds18b20_case__disconnects = 0;
static u16 ds18b20_case__temperatureX16 = 0;

//Whether reset procedure detect presence pulse or not
;

//Last received byte
;

//Last scratchpad
;

//Statistics
;
;
;

//Temperature (must be divided by 16 to convert to degrees)
;
;



static AKAT_FORCE_INLINE void ds18b20_case__ticker() {
//Maintain freshness
    ds18b20_case__updated_deciseconds_ago += AKAT_ONE;

    if (!ds18b20_case__updated_deciseconds_ago) {//We can't go beyond 255
        ds18b20_case__updated_deciseconds_ago -= AKAT_ONE;
    }
}

;





typedef struct {
    u8 (* const get_updated_deciseconds_ago)();
    u8 (* const get_disconnects)();
    u8 (* const get_crc_errors)();
    u16 (* const get_temperatureX16)();
} ds18b20_case_t;

extern ds18b20_case_t const ds18b20_case;

static AKAT_FORCE_INLINE u8 ds18b20_case__get_updated_deciseconds_ago__impl() {
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl
    return ds18b20_case__updated_deciseconds_ago;
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__get_disconnects__impl() {
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl
    return ds18b20_case__disconnects;
#undef get_disconnects__impl
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u8 ds18b20_case__get_crc_errors__impl() {
#define get_crc_errors__impl ds18b20_case__get_crc_errors__impl
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl
    return ds18b20_case__crc_errors;
#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_updated_deciseconds_ago__impl
}
static AKAT_FORCE_INLINE u16 ds18b20_case__get_temperatureX16__impl() {
#define get_crc_errors__impl ds18b20_case__get_crc_errors__impl
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_case__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl
    return ds18b20_case__temperatureX16;
#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
}
#define get_crc_errors__impl ds18b20_case__get_crc_errors__impl
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_case__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl

ds18b20_case_t const ds18b20_case = {.get_updated_deciseconds_ago = &get_updated_deciseconds_ago__impl
                                     ,
                                     .get_disconnects = &get_disconnects__impl
                                             ,
                                     .get_crc_errors = &get_crc_errors__impl
                                             ,
                                     .get_temperatureX16 = &get_temperatureX16__impl
                                    };


#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
#define get_crc_errors__impl ds18b20_case__get_crc_errors__impl
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_case__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl


;

#define get_crc_errors__impl ds18b20_case__get_crc_errors__impl
#define get_disconnects__impl ds18b20_case__get_disconnects__impl
#define get_temperatureX16__impl ds18b20_case__get_temperatureX16__impl
#define get_updated_deciseconds_ago__impl ds18b20_case__get_updated_deciseconds_ago__impl







#undef get_crc_errors__impl
#undef get_disconnects__impl
#undef get_temperatureX16__impl
#undef get_updated_deciseconds_ago__impl
;



;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// USART1 - MH-Z19 CO2 Module

// NOTE: Use to debug the protocol: debug = add_debug_byte
static AKAT_FORCE_INLINE void co2_init() {
//Set baud rate
    const u16 ubrr = akat_cpu_freq_hz() / (9600 * 8L) - 1;
    UBRR1H = ubrr >> 8;
    UBRR1L = ubrr % 256;
    UCSR1A = H(U2X1);
    //Set frame format: 8N1
    UCSR1C = H(UCSZ10) | H(UCSZ11);
    //Enable transmitter, receiver and interrupt for receiver (interrupt for 'byte is received')
    UCSR1B = H(TXEN1) | H(RXEN1) | H(RXCIE1);
}

;





// --- - - - - - - - - - - - RX - - - - - - - -  - - - - - - - --
// Interrupt handler for 'byte is received' event..

static volatile u8 co2_rx_bytes_buf[32] = {};
static volatile u8 co2_rx_overflow_count = 0;
static volatile u8 co2_rx_next_empty_idx = 0;
static volatile u8 co2_rx_next_read_idx = 0;

;
;
;
;
;


ISR(USART1_RX_vect) {
    u8 b = UDR1; // we must read here, no matter what, to clear interrupt flag
    u8 new_next_empty_idx = (co2_rx_next_empty_idx + AKAT_ONE) & (32 - 1);

    if (new_next_empty_idx == co2_rx_next_read_idx) {
        co2_rx_overflow_count += AKAT_ONE;

        // Don't let it overflow!
        if (!co2_rx_overflow_count) {
            co2_rx_overflow_count -= AKAT_ONE;
        }
    } else {
        co2_rx_bytes_buf[co2_rx_next_empty_idx] = b;
        co2_rx_next_empty_idx = new_next_empty_idx;
    }
}

//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// co2(CO2): This thread processes input from co2_rx_bytes_buf that gets populated in ISR

static u8 co2_abc_setup_needed = 0;
static u16 co2_abc_setups = 0;
static u32 co2_deciseconds_until_abc = 0;
static u8 co2_crc_errors = 0;
static u16 co2_concentration = 0;
static u8 co2_temperature = 0;
static u8 co2_updated_deciseconds_ago = 255;
static u8 co2_s = 0;
static u16 co2_u = 0;

;
;
;
;
;
;
;

//https://habr.com/ru/post/401363///and other pages... hard to tell what it is, likely
//s - status
//u - uncorrect minimum value of CO2 measured during previous 24h. Used for ABC (Automatic Baseline Correction).
;
;
;



static AKAT_FORCE_INLINE void co2__ticker() {
//Maintain freshness
    co2_updated_deciseconds_ago += AKAT_ONE;

    if (!co2_updated_deciseconds_ago) {//We can't go beyond 255
        co2_updated_deciseconds_ago -= AKAT_ONE;
    }//Maintain deciseconds until next ABC

    if (co2_deciseconds_until_abc) {
        co2_deciseconds_until_abc -= 1;
    } else {
        co2_abc_setup_needed = 1;
        co2_deciseconds_until_abc = (u32)12 * 60 * 60 * 10; //12 hours
    }
}

;





static u8 co2_reader__akat_coroutine_state = 0;
static u8 co2_reader__dequeued_byte = 0;
static u8 co2_reader__crc = 0;
static u8 co2_reader__command = 0;
static u8 co2_reader__b2 = 0;
static u8 co2_reader__b3 = 0;
static u8 co2_reader__b4 = 0;
static u8 co2_reader__b5 = 0;
static u8 co2_reader__b6 = 0;
static u8 co2_reader__b7 = 0;
static u8 co2_reader__dequeue_byte__akat_coroutine_state = 0;
static u8 co2_reader__dequeue_byte() {
#define akat_coroutine_state co2_reader__dequeue_byte__akat_coroutine_state
#define b2 co2_reader__b2
#define b3 co2_reader__b3
#define b4 co2_reader__b4
#define b5 co2_reader__b5
#define b6 co2_reader__b6
#define b7 co2_reader__b7
#define command co2_reader__command
#define crc co2_reader__crc
#define dequeue_byte co2_reader__dequeue_byte
#define dequeued_byte co2_reader__dequeued_byte
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Wait until there is something to read
        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (!(co2_rx_next_empty_idx != co2_rx_next_read_idx)) {
                AKAT_HOT_CODE;
                return akat_coroutine_state;
            }
        } while (0);

        ;
        //Read byte first, then increment idx!
        dequeued_byte = co2_rx_bytes_buf[co2_rx_next_read_idx];
        co2_rx_next_read_idx = (co2_rx_next_read_idx + 1) & (32 - 1);
        crc += dequeued_byte;
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef b2
#undef b3
#undef b4
#undef b5
#undef b6
#undef b7
#undef command
#undef crc
#undef dequeue_byte
#undef dequeued_byte
}
static AKAT_FORCE_INLINE void co2_reader() {
#define akat_coroutine_state co2_reader__akat_coroutine_state
#define b2 co2_reader__b2
#define b3 co2_reader__b3
#define b4 co2_reader__b4
#define b5 co2_reader__b5
#define b6 co2_reader__b6
#define b7 co2_reader__b7
#define command co2_reader__command
#define crc co2_reader__crc
#define dequeue_byte co2_reader__dequeue_byte
#define dequeued_byte co2_reader__dequeued_byte
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;

    case 9:
        goto akat_coroutine_l_9;

    case 10:
        goto akat_coroutine_l_10;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        ;
        ;
        ;
        //byte of protocol, we don't put them into named variables until CRC is checked
        ;
        ;
        ;
        ;
        ;
        ;

        //Gets byte from co2_rx_bytes_buf buffer.

//- - - - - - - - - - -
        //Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
        while (1) {
            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
try_interpret_as_command:

            if (dequeued_byte == 0xFF) {//0xFF means start of the command...
                //CRC starts calculation only after 0xFF byte
                crc = 0;

                //Read command identifier
                do {
                    akat_coroutine_state = 3;
akat_coroutine_l_3:

                    if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;

                //0x86 - Read CO2 concentration.. 0x79 is ack for ABC setup
                if (dequeued_byte == 0x86 || dequeued_byte == 0x79) {
                    command = dequeued_byte;

                    do {
                        akat_coroutine_state = 4;
akat_coroutine_l_4:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b2 = dequeued_byte;

                    do {
                        akat_coroutine_state = 5;
akat_coroutine_l_5:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b3 = dequeued_byte;

                    do {
                        akat_coroutine_state = 6;
akat_coroutine_l_6:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b4 = dequeued_byte;

                    do {
                        akat_coroutine_state = 7;
akat_coroutine_l_7:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b5 = dequeued_byte;

                    do {
                        akat_coroutine_state = 8;
akat_coroutine_l_8:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b6 = dequeued_byte;

                    do {
                        akat_coroutine_state = 9;
akat_coroutine_l_9:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    b7 = dequeued_byte;

                    //Check CRC
                    do {
                        akat_coroutine_state = 10;
akat_coroutine_l_10:

                        if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    crc -= dequeued_byte;
                    crc = 0xFF - crc + 1;

                    if (dequeued_byte == crc) {
                        if (command == 0x86) {//CO2 read
                            if (b4 < 90 && b4 > 30) {
                                co2_concentration = (((u16)b2) << 8) + b3;
                                co2_temperature = b4 - 40;
                                co2_updated_deciseconds_ago = 0;
                            }

                            co2_s = b5;
                            co2_u = (((u16)b6) << 8) + b7;
                        } else if (command == 0x79) {//ABC setup result
                            co2_abc_setup_needed = 0;
                            co2_abc_setups += 1;
                        }
                    } else {//CRC doesn't match
                        co2_crc_errors += 1;
                    }
                } else {//Wrong command... may be 0xFF? Then we must try to use it as start of command.
                    goto try_interpret_as_command;
                }
            } else {//Unknown byte
            }
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_END;
akat_coroutine_l_end:
    return;
#undef akat_coroutine_state
#undef b2
#undef b3
#undef b4
#undef b5
#undef b6
#undef b7
#undef command
#undef crc
#undef dequeue_byte
#undef dequeued_byte
}

;






// --- - - - - - - - - - - - TX - - - - - - - - - - - - - - -

static u8 co2_command_countdown = 0;

;
;



static AKAT_FORCE_INLINE void co2_ticker() {
    if (co2_command_countdown) {
        co2_command_countdown -= 1;
    }
}

;





static u8 co2_writer__akat_coroutine_state = 0;
static u8 co2_writer__byte_to_send = 0;
static u8 co2_writer__send_byte__akat_coroutine_state = 0;
static u8 co2_writer__send_byte() {
#define akat_coroutine_state co2_writer__send_byte__akat_coroutine_state
#define byte_to_send co2_writer__byte_to_send
#define send_byte co2_writer__send_byte
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Wait until USART0 is ready to transmit next byte
        //from 'byte_to_send';
        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (!(UCSR1A & H(UDRE1))) {
                AKAT_HOT_CODE;
                return akat_coroutine_state;
            }
        } while (0);

        ;
        UDR1 = byte_to_send;
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef send_byte
}
static u8 co2_writer__send_read_concentration_command__akat_coroutine_state = 0;
static u8 co2_writer__send_read_concentration_command() {
#define akat_coroutine_state co2_writer__send_read_concentration_command__akat_coroutine_state
#define byte_to_send co2_writer__byte_to_send
#define send_byte co2_writer__send_byte
#define send_read_concentration_command co2_writer__send_read_concentration_command
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;

    case 9:
        goto akat_coroutine_l_9;

    case 10:
        goto akat_coroutine_l_10;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Send command sequence
        byte_to_send = 0xFF;

        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Header
        byte_to_send = 0x01;

        do {
            akat_coroutine_state = 3;
akat_coroutine_l_3:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Sensor #1
        byte_to_send = 0x86;

        do {
            akat_coroutine_state = 4;
akat_coroutine_l_4:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Command (Read gas concentration)
        //5 times zero
        byte_to_send = 0x00;

        do {
            akat_coroutine_state = 5;
akat_coroutine_l_5:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 6;
akat_coroutine_l_6:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 7;
akat_coroutine_l_7:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 8;
akat_coroutine_l_8:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 9;
akat_coroutine_l_9:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;
        byte_to_send = 0x79;

        do {
            akat_coroutine_state = 10;
akat_coroutine_l_10:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //CRC
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef send_byte
#undef send_read_concentration_command
}
static u8 co2_writer__setup_abc__akat_coroutine_state = 0;
static u8 co2_writer__setup_abc() {
#define akat_coroutine_state co2_writer__setup_abc__akat_coroutine_state
#define byte_to_send co2_writer__byte_to_send
#define send_byte co2_writer__send_byte
#define send_read_concentration_command co2_writer__send_read_concentration_command
#define setup_abc co2_writer__setup_abc
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;

    case 9:
        goto akat_coroutine_l_9;

    case 10:
        goto akat_coroutine_l_10;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Send command sequence
        byte_to_send = 0xFF;

        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Header
        byte_to_send = 0x01;

        do {
            akat_coroutine_state = 3;
akat_coroutine_l_3:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Sensor #1
        byte_to_send = 0x79;

        do {
            akat_coroutine_state = 4;
akat_coroutine_l_4:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //Command (ON/OFF Self-calibration for zero point)

        if (0) {
            byte_to_send = 0xA0;
        } else {
            byte_to_send = 0x00;
        }

        do {
            akat_coroutine_state = 5;
akat_coroutine_l_5:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;
        byte_to_send = 0x00;

        do {
            akat_coroutine_state = 6;
akat_coroutine_l_6:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 7;
akat_coroutine_l_7:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 8;
akat_coroutine_l_8:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        do {
            akat_coroutine_state = 9;
akat_coroutine_l_9:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (0) {
            byte_to_send = 0xE6;
        } else {
            byte_to_send = 0x86;
        }

        do {
            akat_coroutine_state = 10;
akat_coroutine_l_10:

            if (send_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ; //CRC
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef send_byte
#undef send_read_concentration_command
#undef setup_abc
}
static AKAT_FORCE_INLINE void co2_writer() {
#define akat_coroutine_state co2_writer__akat_coroutine_state
#define byte_to_send co2_writer__byte_to_send
#define send_byte co2_writer__send_byte
#define send_read_concentration_command co2_writer__send_read_concentration_command
#define setup_abc co2_writer__setup_abc
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        ;

        while (1) { //Wait until it's time to send the command sequence
            //This counter will be incremented every 0.1 second in the X_EVERY_DECISECOND above
            co2_command_countdown = 10; //No need to ask more often than that

            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (!(co2_command_countdown == 0)) {
                    AKAT_HOT_CODE;
                    return ;
                }
            } while (0);

            ;

            if (co2_abc_setup_needed && co2_updated_deciseconds_ago != 255) {//Seems like sensor is responding to our commands and ABC (Automatic Baseline Correction) setup is needed
                //We do co2_abc_setup_needed = 0 when we receive ack-response.
                do {
                    akat_coroutine_state = 3;
akat_coroutine_l_3:

                    if (setup_abc() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;
            } else {
                do {
                    akat_coroutine_state = 4;
akat_coroutine_l_4:

                    if (send_read_concentration_command() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;
            }
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_END;
akat_coroutine_l_end:
    return;
#undef akat_coroutine_state
#undef byte_to_send
#undef send_byte
#undef send_read_concentration_command
#undef setup_abc
}

;






// --- - - - - - - - - - - - Interface - - - - - - - - - - - - - - -

typedef struct {
    u8 (* const get_rx_overflow_count)();
    u8 (* const get_crc_errors)();
    u16 (* const get_concentration)();
    u16 (* const get_u)();
    u8 (* const get_s)();
    u8 (* const get_temperature)();
    u16 (* const get_abc_setups)();
    u8 (* const get_updated_deciseconds_ago)();
} co2_t;

extern co2_t const co2;

static AKAT_FORCE_INLINE u8 co2__get_rx_overflow_count__impl() {
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
    return co2_rx_overflow_count;
#undef get_rx_overflow_count__impl
}
static AKAT_FORCE_INLINE u8 co2__get_crc_errors__impl() {
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
    return co2_crc_errors;
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
}
static AKAT_FORCE_INLINE u16 co2__get_concentration__impl() {
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
    return co2_concentration;
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
}
static AKAT_FORCE_INLINE u16 co2__get_u__impl() {
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_u__impl co2__get_u__impl
    return co2_u;
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_u__impl
}
static AKAT_FORCE_INLINE u8 co2__get_s__impl() {
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_u__impl co2__get_u__impl
    return co2_s;
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_u__impl
}
static AKAT_FORCE_INLINE u8 co2__get_temperature__impl() {
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
    return co2_temperature;
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_temperature__impl
#undef get_u__impl
}
static AKAT_FORCE_INLINE u16 co2__get_abc_setups__impl() {
#define get_abc_setups__impl co2__get_abc_setups__impl
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
    return co2_abc_setups;
#undef get_abc_setups__impl
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_temperature__impl
#undef get_u__impl
}
static AKAT_FORCE_INLINE u8 co2__get_updated_deciseconds_ago__impl() {
#define get_abc_setups__impl co2__get_abc_setups__impl
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
#define get_updated_deciseconds_ago__impl co2__get_updated_deciseconds_ago__impl
    return co2_updated_deciseconds_ago;
#undef get_abc_setups__impl
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_temperature__impl
#undef get_u__impl
#undef get_updated_deciseconds_ago__impl
}
#define get_abc_setups__impl co2__get_abc_setups__impl
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
#define get_updated_deciseconds_ago__impl co2__get_updated_deciseconds_ago__impl

co2_t const co2 = {.get_rx_overflow_count = &get_rx_overflow_count__impl
                   ,
                   .get_crc_errors = &get_crc_errors__impl
                                     ,
                   .get_concentration = &get_concentration__impl
                                        ,
                   .get_u = &get_u__impl
                            ,
                   .get_s = &get_s__impl
                            ,
                   .get_temperature = &get_temperature__impl
                                      ,
                   .get_abc_setups = &get_abc_setups__impl
                                     ,
                   .get_updated_deciseconds_ago = &get_updated_deciseconds_ago__impl
                  };


#undef get_abc_setups__impl
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_temperature__impl
#undef get_u__impl
#undef get_updated_deciseconds_ago__impl
#define get_abc_setups__impl co2__get_abc_setups__impl
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
#define get_updated_deciseconds_ago__impl co2__get_updated_deciseconds_ago__impl


;

#define get_abc_setups__impl co2__get_abc_setups__impl
#define get_concentration__impl co2__get_concentration__impl
#define get_crc_errors__impl co2__get_crc_errors__impl
#define get_rx_overflow_count__impl co2__get_rx_overflow_count__impl
#define get_s__impl co2__get_s__impl
#define get_temperature__impl co2__get_temperature__impl
#define get_u__impl co2__get_u__impl
#define get_updated_deciseconds_ago__impl co2__get_updated_deciseconds_ago__impl











#undef get_abc_setups__impl
#undef get_concentration__impl
#undef get_crc_errors__impl
#undef get_rx_overflow_count__impl
#undef get_s__impl
#undef get_temperature__impl
#undef get_u__impl
#undef get_updated_deciseconds_ago__impl
;



;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// USART0 - Serial interface over USB Connection

static AKAT_FORCE_INLINE void usart0_init() {
//Set baud rate
    const u16 ubrr = akat_cpu_freq_hz() / (AK_USART0_BAUD_RATE * 8L) - 1;
    UBRR0H = ubrr >> 8;
    UBRR0L = ubrr % 256;
    UCSR0A = H(U2X0);
    //Set frame format
    UCSR0C = AK_USART0_FRAME_FORMAT;
    //Enable transmitter, receiver and interrupt for receiver (interrupt for 'byte is received')
    UCSR0B = H(TXEN0) | H(RXEN0) | H(RXCIE0);
}

;





// ----------------------------------------------------------------
// USART0(USB): Interrupt handler for 'byte is received' event..

static volatile u8 usart0_rx_bytes_buf[AK_USART0_RX_BUF_SIZE] = {};
static volatile u8 usart0_rx_overflow_count = 0;
static volatile u8 usart0_rx_next_empty_idx = 0;
static volatile u8 usart0_rx_next_read_idx = 0;

;
;
;
;
;


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
static u8 usart0_writer__akat_coroutine_state = 0;
static u8 usart0_writer__crc = 0;
static u8 usart0_writer__byte_to_send = 0;
static u8 usart0_writer__u8_to_format_and_send = 0;
static u16 usart0_writer__u16_to_format_and_send = 0;
static u32 usart0_writer__u32_to_format_and_send = 0;
static u8 usart0_writer__send_byte__akat_coroutine_state = 0;
static u8 usart0_writer__send_byte() {
#define akat_coroutine_state usart0_writer__send_byte__akat_coroutine_state
#define byte_to_send usart0_writer__byte_to_send
#define crc usart0_writer__crc
#define send_byte usart0_writer__send_byte
#define u16_to_format_and_send usart0_writer__u16_to_format_and_send
#define u32_to_format_and_send usart0_writer__u32_to_format_and_send
#define u8_to_format_and_send usart0_writer__u8_to_format_and_send
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Wait until USART0 is ready to transmit next byte
        //from 'byte_to_send';
        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (!(UCSR0A & H(UDRE0))) {
                AKAT_HOT_CODE;
                return akat_coroutine_state;
            }
        } while (0);

        ;
        UDR0 = byte_to_send;
        crc = akat_crc_add(crc, byte_to_send);
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef crc
#undef send_byte
#undef u16_to_format_and_send
#undef u32_to_format_and_send
#undef u8_to_format_and_send
}
static u8 usart0_writer__format_and_send_u8__akat_coroutine_state = 0;
static u8 usart0_writer__format_and_send_u8() {
#define akat_coroutine_state usart0_writer__format_and_send_u8__akat_coroutine_state
#define byte_to_send usart0_writer__byte_to_send
#define crc usart0_writer__crc
#define format_and_send_u8 usart0_writer__format_and_send_u8
#define send_byte usart0_writer__send_byte
#define u16_to_format_and_send usart0_writer__u16_to_format_and_send
#define u32_to_format_and_send usart0_writer__u32_to_format_and_send
#define u8_to_format_and_send usart0_writer__u8_to_format_and_send
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        if (u8_to_format_and_send) {
            u8 h = u8_to_format_and_send / 16;

            if (h) {
                byte_to_send = HEX[h];

                do {
                    akat_coroutine_state = 2;
akat_coroutine_l_2:

                    if (send_byte() != AKAT_COROUTINE_S_START) {
                        return akat_coroutine_state;
                    }
                } while (0);

                ;
            }

            u8 i = u8_to_format_and_send & 15;
            byte_to_send = HEX[i];

            do {
                akat_coroutine_state = 3;
akat_coroutine_l_3:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef crc
#undef format_and_send_u8
#undef send_byte
#undef u16_to_format_and_send
#undef u32_to_format_and_send
#undef u8_to_format_and_send
}
static u8 usart0_writer__format_and_send_u16__akat_coroutine_state = 0;
static u8 usart0_writer__format_and_send_u16() {
#define akat_coroutine_state usart0_writer__format_and_send_u16__akat_coroutine_state
#define byte_to_send usart0_writer__byte_to_send
#define crc usart0_writer__crc
#define format_and_send_u16 usart0_writer__format_and_send_u16
#define format_and_send_u8 usart0_writer__format_and_send_u8
#define send_byte usart0_writer__send_byte
#define u16_to_format_and_send usart0_writer__u16_to_format_and_send
#define u32_to_format_and_send usart0_writer__u32_to_format_and_send
#define u8_to_format_and_send usart0_writer__u8_to_format_and_send
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        u8_to_format_and_send = (u8)(u16_to_format_and_send / 256);

        if (u8_to_format_and_send) {
            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            u8_to_format_and_send = (u8)u16_to_format_and_send;
            byte_to_send = HEX[u8_to_format_and_send / 16];

            do {
                akat_coroutine_state = 3;
akat_coroutine_l_3:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            byte_to_send = HEX[u8_to_format_and_send & 15];

            do {
                akat_coroutine_state = 4;
akat_coroutine_l_4:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
        } else {
            u8_to_format_and_send = (u8)u16_to_format_and_send;

            do {
                akat_coroutine_state = 5;
akat_coroutine_l_5:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef crc
#undef format_and_send_u16
#undef format_and_send_u8
#undef send_byte
#undef u16_to_format_and_send
#undef u32_to_format_and_send
#undef u8_to_format_and_send
}
static u8 usart0_writer__format_and_send_u32__akat_coroutine_state = 0;
static u8 usart0_writer__format_and_send_u32() {
#define akat_coroutine_state usart0_writer__format_and_send_u32__akat_coroutine_state
#define byte_to_send usart0_writer__byte_to_send
#define crc usart0_writer__crc
#define format_and_send_u16 usart0_writer__format_and_send_u16
#define format_and_send_u32 usart0_writer__format_and_send_u32
#define format_and_send_u8 usart0_writer__format_and_send_u8
#define send_byte usart0_writer__send_byte
#define u16_to_format_and_send usart0_writer__u16_to_format_and_send
#define u32_to_format_and_send usart0_writer__u32_to_format_and_send
#define u8_to_format_and_send usart0_writer__u8_to_format_and_send
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        u16_to_format_and_send = (u16)(u32_to_format_and_send >> 16);

        if (u16_to_format_and_send) {
            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            u16_to_format_and_send = (u16)u32_to_format_and_send;
            u8_to_format_and_send = (u8)(u16_to_format_and_send / 256);
            byte_to_send = HEX[u8_to_format_and_send / 16];

            do {
                akat_coroutine_state = 3;
akat_coroutine_l_3:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            byte_to_send = HEX[u8_to_format_and_send & 15];

            do {
                akat_coroutine_state = 4;
akat_coroutine_l_4:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            u8_to_format_and_send = (u8)u16_to_format_and_send;
            byte_to_send = HEX[u8_to_format_and_send / 16];

            do {
                akat_coroutine_state = 5;
akat_coroutine_l_5:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
            byte_to_send = HEX[u8_to_format_and_send & 15];

            do {
                akat_coroutine_state = 6;
akat_coroutine_l_6:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
        } else {
            u16_to_format_and_send = (u16)u32_to_format_and_send;

            do {
                akat_coroutine_state = 7;
akat_coroutine_l_7:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef crc
#undef format_and_send_u16
#undef format_and_send_u32
#undef format_and_send_u8
#undef send_byte
#undef u16_to_format_and_send
#undef u32_to_format_and_send
#undef u8_to_format_and_send
}
static AKAT_FORCE_INLINE void usart0_writer() {
#define akat_coroutine_state usart0_writer__akat_coroutine_state
#define byte_to_send usart0_writer__byte_to_send
#define crc usart0_writer__crc
#define format_and_send_u16 usart0_writer__format_and_send_u16
#define format_and_send_u32 usart0_writer__format_and_send_u32
#define format_and_send_u8 usart0_writer__format_and_send_u8
#define send_byte usart0_writer__send_byte
#define u16_to_format_and_send usart0_writer__u16_to_format_and_send
#define u32_to_format_and_send usart0_writer__u32_to_format_and_send
#define u8_to_format_and_send usart0_writer__u8_to_format_and_send
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;

    case 9:
        goto akat_coroutine_l_9;

    case 10:
        goto akat_coroutine_l_10;

    case 11:
        goto akat_coroutine_l_11;

    case 12:
        goto akat_coroutine_l_12;

    case 13:
        goto akat_coroutine_l_13;

    case 14:
        goto akat_coroutine_l_14;

    case 15:
        goto akat_coroutine_l_15;

    case 16:
        goto akat_coroutine_l_16;

    case 17:
        goto akat_coroutine_l_17;

    case 18:
        goto akat_coroutine_l_18;

    case 19:
        goto akat_coroutine_l_19;

    case 20:
        goto akat_coroutine_l_20;

    case 21:
        goto akat_coroutine_l_21;

    case 22:
        goto akat_coroutine_l_22;

    case 23:
        goto akat_coroutine_l_23;

    case 24:
        goto akat_coroutine_l_24;

    case 25:
        goto akat_coroutine_l_25;

    case 26:
        goto akat_coroutine_l_26;

    case 27:
        goto akat_coroutine_l_27;

    case 28:
        goto akat_coroutine_l_28;

    case 29:
        goto akat_coroutine_l_29;

    case 30:
        goto akat_coroutine_l_30;

    case 31:
        goto akat_coroutine_l_31;

    case 32:
        goto akat_coroutine_l_32;

    case 33:
        goto akat_coroutine_l_33;

    case 34:
        goto akat_coroutine_l_34;

    case 35:
        goto akat_coroutine_l_35;

    case 36:
        goto akat_coroutine_l_36;

    case 37:
        goto akat_coroutine_l_37;

    case 38:
        goto akat_coroutine_l_38;

    case 39:
        goto akat_coroutine_l_39;

    case 40:
        goto akat_coroutine_l_40;

    case 41:
        goto akat_coroutine_l_41;

    case 42:
        goto akat_coroutine_l_42;

    case 43:
        goto akat_coroutine_l_43;

    case 44:
        goto akat_coroutine_l_44;

    case 45:
        goto akat_coroutine_l_45;

    case 46:
        goto akat_coroutine_l_46;

    case 47:
        goto akat_coroutine_l_47;

    case 48:
        goto akat_coroutine_l_48;

    case 49:
        goto akat_coroutine_l_49;

    case 50:
        goto akat_coroutine_l_50;

    case 51:
        goto akat_coroutine_l_51;

    case 52:
        goto akat_coroutine_l_52;

    case 53:
        goto akat_coroutine_l_53;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //---- All variable in the thread must be static (green threads requirement)
        ;
        ;
        ;
        ;
        ;

        //---- Subroutines can yield unlike functions

//---- Macro that writes the given status into UART
        //We also write some humand readable description of the protocol
        //also stuff to distinguish protocol versions and generate typescript parser code

        /* Defined new macro with name WRITE_STATUS  *///- - - - - - - - - - -

        //Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
        while (1) { //---- - - - - -- - - - - - - -
            //Write debug if there is some
            if (debug_next_empty_idx != debug_next_read_idx) {
                while (debug_next_empty_idx != debug_next_read_idx) {
                    byte_to_send = '>';

                    do {
                        akat_coroutine_state = 2;
akat_coroutine_l_2:

                        if (send_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    //Read byte first, then increment idx!
                    u8_to_format_and_send = debug_bytes_buf[debug_next_read_idx];
                    debug_next_read_idx = (debug_next_read_idx + 1) & (AK_DEBUG_BUF_SIZE - 1);

                    do {
                        akat_coroutine_state = 3;
akat_coroutine_l_3:

                        if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                }

                byte_to_send = '\r';

                do {
                    akat_coroutine_state = 4;
akat_coroutine_l_4:

                    if (send_byte() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;
                byte_to_send = '\n';

                do {
                    akat_coroutine_state = 5;
akat_coroutine_l_5:

                    if (send_byte() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;
            }//----  - - - - -- - - - - -

            crc = 0;
            //WRITE_STATUS(name for documentation, 1-character id for protocol, type1 val1, type2 val2, ...)
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 6;
akat_coroutine_l_6:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = 'A';

            do {
                akat_coroutine_state = 7;
akat_coroutine_l_7:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: A1: Misc: u32 uptime_deciseconds
              TS_PROTO_TYPE: "u32 uptime_deciseconds": number,
              TS_PROTO_ASSIGN: "u32 uptime_deciseconds": vals["A1"],
            */
            u32_to_format_and_send = uptime_deciseconds;

            do {
                akat_coroutine_state = 8;
akat_coroutine_l_8:

                if (format_and_send_u32() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 9;
akat_coroutine_l_9:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: A2: Misc: u8 debug_overflow_count
              TS_PROTO_TYPE: "u8 debug_overflow_count": number,
              TS_PROTO_ASSIGN: "u8 debug_overflow_count": vals["A2"],
            */
            u8_to_format_and_send = debug_overflow_count;

            do {
                akat_coroutine_state = 10;
akat_coroutine_l_10:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 11;
akat_coroutine_l_11:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: A3: Misc: u8 usart0_rx_overflow_count
              TS_PROTO_TYPE: "u8 usart0_rx_overflow_count": number,
              TS_PROTO_ASSIGN: "u8 usart0_rx_overflow_count": vals["A3"],
            */
            u8_to_format_and_send = usart0_rx_overflow_count;

            do {
                akat_coroutine_state = 12;
akat_coroutine_l_12:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            ;
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 13;
akat_coroutine_l_13:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = 'B';

            do {
                akat_coroutine_state = 14;
akat_coroutine_l_14:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: B1: Aquarium temperature sensor: u8 ds18b20_aqua.get_crc_errors()
              TS_PROTO_TYPE: "u8 ds18b20_aqua.get_crc_errors()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_aqua.get_crc_errors()": vals["B1"],
            */
            u8_to_format_and_send = ds18b20_aqua.get_crc_errors();

            do {
                akat_coroutine_state = 15;
akat_coroutine_l_15:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 16;
akat_coroutine_l_16:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: B2: Aquarium temperature sensor: u8 ds18b20_aqua.get_disconnects()
              TS_PROTO_TYPE: "u8 ds18b20_aqua.get_disconnects()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_aqua.get_disconnects()": vals["B2"],
            */
            u8_to_format_and_send = ds18b20_aqua.get_disconnects();

            do {
                akat_coroutine_state = 17;
akat_coroutine_l_17:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 18;
akat_coroutine_l_18:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: B3: Aquarium temperature sensor: u16 ds18b20_aqua.get_temperatureX16()
              TS_PROTO_TYPE: "u16 ds18b20_aqua.get_temperatureX16()": number,
              TS_PROTO_ASSIGN: "u16 ds18b20_aqua.get_temperatureX16()": vals["B3"],
            */
            u16_to_format_and_send = ds18b20_aqua.get_temperatureX16();

            do {
                akat_coroutine_state = 19;
akat_coroutine_l_19:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 20;
akat_coroutine_l_20:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: B4: Aquarium temperature sensor: u8 ds18b20_aqua.get_updated_deciseconds_ago()
              TS_PROTO_TYPE: "u8 ds18b20_aqua.get_updated_deciseconds_ago()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_aqua.get_updated_deciseconds_ago()": vals["B4"],
            */
            u8_to_format_and_send = ds18b20_aqua.get_updated_deciseconds_ago();

            do {
                akat_coroutine_state = 21;
akat_coroutine_l_21:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            ;
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 22;
akat_coroutine_l_22:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = 'C';

            do {
                akat_coroutine_state = 23;
akat_coroutine_l_23:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: C1: Case temperature sensor: u8 ds18b20_case.get_crc_errors()
              TS_PROTO_TYPE: "u8 ds18b20_case.get_crc_errors()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_case.get_crc_errors()": vals["C1"],
            */
            u8_to_format_and_send = ds18b20_case.get_crc_errors();

            do {
                akat_coroutine_state = 24;
akat_coroutine_l_24:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 25;
akat_coroutine_l_25:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: C2: Case temperature sensor: u8 ds18b20_case.get_disconnects()
              TS_PROTO_TYPE: "u8 ds18b20_case.get_disconnects()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_case.get_disconnects()": vals["C2"],
            */
            u8_to_format_and_send = ds18b20_case.get_disconnects();

            do {
                akat_coroutine_state = 26;
akat_coroutine_l_26:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 27;
akat_coroutine_l_27:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: C3: Case temperature sensor: u16 ds18b20_case.get_temperatureX16()
              TS_PROTO_TYPE: "u16 ds18b20_case.get_temperatureX16()": number,
              TS_PROTO_ASSIGN: "u16 ds18b20_case.get_temperatureX16()": vals["C3"],
            */
            u16_to_format_and_send = ds18b20_case.get_temperatureX16();

            do {
                akat_coroutine_state = 28;
akat_coroutine_l_28:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 29;
akat_coroutine_l_29:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: C4: Case temperature sensor: u8 ds18b20_case.get_updated_deciseconds_ago()
              TS_PROTO_TYPE: "u8 ds18b20_case.get_updated_deciseconds_ago()": number,
              TS_PROTO_ASSIGN: "u8 ds18b20_case.get_updated_deciseconds_ago()": vals["C4"],
            */
            u8_to_format_and_send = ds18b20_case.get_updated_deciseconds_ago();

            do {
                akat_coroutine_state = 30;
akat_coroutine_l_30:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            ;
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 31;
akat_coroutine_l_31:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = 'D';

            do {
                akat_coroutine_state = 32;
akat_coroutine_l_32:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D1: CO2 sensor: u8 co2.get_rx_overflow_count()
              TS_PROTO_TYPE: "u8 co2.get_rx_overflow_count()": number,
              TS_PROTO_ASSIGN: "u8 co2.get_rx_overflow_count()": vals["D1"],
            */
            u8_to_format_and_send = co2.get_rx_overflow_count();

            do {
                akat_coroutine_state = 33;
akat_coroutine_l_33:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 34;
akat_coroutine_l_34:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D2: CO2 sensor: u8 co2.get_crc_errors()
              TS_PROTO_TYPE: "u8 co2.get_crc_errors()": number,
              TS_PROTO_ASSIGN: "u8 co2.get_crc_errors()": vals["D2"],
            */
            u8_to_format_and_send = co2.get_crc_errors();

            do {
                akat_coroutine_state = 35;
akat_coroutine_l_35:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 36;
akat_coroutine_l_36:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D3: CO2 sensor: u16 co2.get_abc_setups()
              TS_PROTO_TYPE: "u16 co2.get_abc_setups()": number,
              TS_PROTO_ASSIGN: "u16 co2.get_abc_setups()": vals["D3"],
            */
            u16_to_format_and_send = co2.get_abc_setups();

            do {
                akat_coroutine_state = 37;
akat_coroutine_l_37:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 38;
akat_coroutine_l_38:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D4: CO2 sensor: u16 co2.get_concentration()
              TS_PROTO_TYPE: "u16 co2.get_concentration()": number,
              TS_PROTO_ASSIGN: "u16 co2.get_concentration()": vals["D4"],
            */
            u16_to_format_and_send = co2.get_concentration();

            do {
                akat_coroutine_state = 39;
akat_coroutine_l_39:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 40;
akat_coroutine_l_40:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D5: CO2 sensor: u8 co2.get_temperature()
              TS_PROTO_TYPE: "u8 co2.get_temperature()": number,
              TS_PROTO_ASSIGN: "u8 co2.get_temperature()": vals["D5"],
            */
            u8_to_format_and_send = co2.get_temperature();

            do {
                akat_coroutine_state = 41;
akat_coroutine_l_41:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 42;
akat_coroutine_l_42:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D6: CO2 sensor: u8 co2.get_s()
              TS_PROTO_TYPE: "u8 co2.get_s()": number,
              TS_PROTO_ASSIGN: "u8 co2.get_s()": vals["D6"],
            */
            u8_to_format_and_send = co2.get_s();

            do {
                akat_coroutine_state = 43;
akat_coroutine_l_43:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 44;
akat_coroutine_l_44:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D7: CO2 sensor: u16 co2.get_u()
              TS_PROTO_TYPE: "u16 co2.get_u()": number,
              TS_PROTO_ASSIGN: "u16 co2.get_u()": vals["D7"],
            */
            u16_to_format_and_send = co2.get_u();

            do {
                akat_coroutine_state = 45;
akat_coroutine_l_45:

                if (format_and_send_u16() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = ',';

            do {
                akat_coroutine_state = 46;
akat_coroutine_l_46:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            /*
              COMMPROTO: D8: CO2 sensor: u8 co2.get_updated_deciseconds_ago()
              TS_PROTO_TYPE: "u8 co2.get_updated_deciseconds_ago()": number,
              TS_PROTO_ASSIGN: "u8 co2.get_updated_deciseconds_ago()": vals["D8"],
            */
            u8_to_format_and_send = co2.get_updated_deciseconds_ago();

            do {
                akat_coroutine_state = 47;
akat_coroutine_l_47:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            ;
            //Protocol version
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 48;
akat_coroutine_l_48:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            u8_to_format_and_send = 0x3b;

            do {
                akat_coroutine_state = 49;
akat_coroutine_l_49:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            //Done writing status, send: CRC\r\n
            byte_to_send = ' ';

            do {
                akat_coroutine_state = 50;
akat_coroutine_l_50:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            u8_to_format_and_send = crc;

            do {
                akat_coroutine_state = 51;
akat_coroutine_l_51:

                if (format_and_send_u8() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            //Newline
            byte_to_send = '\r';

            do {
                akat_coroutine_state = 52;
akat_coroutine_l_52:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
            byte_to_send = '\n';

            do {
                akat_coroutine_state = 53;
akat_coroutine_l_53:

                if (send_byte() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_END;
akat_coroutine_l_end:
    return;
#undef akat_coroutine_state
#undef byte_to_send
#undef crc
#undef format_and_send_u16
#undef format_and_send_u32
#undef format_and_send_u8
#undef send_byte
#undef u16_to_format_and_send
#undef u32_to_format_and_send
#undef u8_to_format_and_send
}

;






// ---------------------------------------------------------------------------------
// USART0(USB): This thread processes input from usart0_rx_bytes_buf that gets populated in ISR

static u8 usart0_reader__akat_coroutine_state = 0;
static u8 usart0_reader__command_code = 0;
static u8 usart0_reader__command_arg = 0;
static u8 usart0_reader__read_command__akat_coroutine_state = 0;
static u8 usart0_reader__read_command__dequeued_byte = 0;
static u8 usart0_reader__read_command__command_arg_copy = 0;
static u8 usart0_reader__read_command__dequeue_byte__akat_coroutine_state = 0;
static u8 usart0_reader__read_command__dequeue_byte() {
#define akat_coroutine_state usart0_reader__read_command__dequeue_byte__akat_coroutine_state
#define command_arg usart0_reader__command_arg
#define command_arg_copy usart0_reader__read_command__command_arg_copy
#define command_code usart0_reader__command_code
#define dequeue_byte usart0_reader__read_command__dequeue_byte
#define dequeued_byte usart0_reader__read_command__dequeued_byte
#define read_command usart0_reader__read_command
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Wait until there is something to read
        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (!(usart0_rx_next_empty_idx != usart0_rx_next_read_idx)) {
                AKAT_HOT_CODE;
                return akat_coroutine_state;
            }
        } while (0);

        ;
        //Read byte first, then increment idx!
        dequeued_byte = usart0_rx_bytes_buf[usart0_rx_next_read_idx];
        usart0_rx_next_read_idx = (usart0_rx_next_read_idx + 1) & (AK_USART0_RX_BUF_SIZE - 1);
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef command_arg
#undef command_arg_copy
#undef command_code
#undef dequeue_byte
#undef dequeued_byte
#undef read_command
}
static u8 usart0_reader__read_command__read_arg_and_dequeue__akat_coroutine_state = 0;
static u8 usart0_reader__read_command__read_arg_and_dequeue() {
#define akat_coroutine_state usart0_reader__read_command__read_arg_and_dequeue__akat_coroutine_state
#define command_arg usart0_reader__command_arg
#define command_arg_copy usart0_reader__read_command__command_arg_copy
#define command_code usart0_reader__command_code
#define dequeue_byte usart0_reader__read_command__dequeue_byte
#define dequeued_byte usart0_reader__read_command__dequeued_byte
#define read_arg_and_dequeue usart0_reader__read_command__read_arg_and_dequeue
#define read_command usart0_reader__read_command
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        command_arg = 0;

        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (dequeued_byte >= '0' && dequeued_byte <= '9') {
            command_arg = dequeued_byte - '0';

            do {
                akat_coroutine_state = 3;
akat_coroutine_l_3:

                if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                    return akat_coroutine_state;
                }
            } while (0);

            ;

            if (dequeued_byte >= '0' && dequeued_byte <= '9') {
                command_arg = command_arg * 10 + (dequeued_byte - '0');

                do {
                    akat_coroutine_state = 4;
akat_coroutine_l_4:

                    if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                        return akat_coroutine_state;
                    }
                } while (0);

                ;

                if (dequeued_byte >= '0' && dequeued_byte <= '9') {
                    if (command_arg < 25 || (command_arg == 25 && dequeued_byte <= '5')) {
                        command_arg = command_arg * 10 + (dequeued_byte - '0');

                        do {
                            akat_coroutine_state = 5;
akat_coroutine_l_5:

                            if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                                return akat_coroutine_state;
                            }
                        } while (0);

                        ;
                    }
                }
            }
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef command_arg
#undef command_arg_copy
#undef command_code
#undef dequeue_byte
#undef dequeued_byte
#undef read_arg_and_dequeue
#undef read_command
}
static u8 usart0_reader__read_command() {
#define akat_coroutine_state usart0_reader__read_command__akat_coroutine_state
#define command_arg usart0_reader__command_arg
#define command_arg_copy usart0_reader__read_command__command_arg_copy
#define command_code usart0_reader__command_code
#define dequeue_byte usart0_reader__read_command__dequeue_byte
#define dequeued_byte usart0_reader__read_command__dequeued_byte
#define read_arg_and_dequeue usart0_reader__read_command__read_arg_and_dequeue
#define read_command usart0_reader__read_command
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        ;
        ;
        //Gets byte from usart0_rx_bytes_buf buffer.
//Read arg into command_arg, leaves byte after arg in the dqeueued_byte variable
        //so the caller must process it as well upon return!
command_reading_start:

        //Read opening bracket
        do {
            akat_coroutine_state = 2;
akat_coroutine_l_2:

            if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (dequeued_byte != '<') {
            goto command_reading_start;
        }//Read command code

        //Verify that code is really a code letter
        do {
            akat_coroutine_state = 3;
akat_coroutine_l_3:

            if (dequeue_byte() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (dequeued_byte < 'A' || dequeued_byte > 'Z') {
            goto command_reading_start;
        }

        command_code = dequeued_byte;

        //Read arg and save it as copy, note that read_arg aborts
        //when it either read foruth character in a row or a non digit character
        //so we have to process it (dequeued_byte) when the call to read_arg returns.
        //Verify that stuff that comes after the arg is a command code again!
        do {
            akat_coroutine_state = 4;
akat_coroutine_l_4:

            if (read_arg_and_dequeue() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (dequeued_byte != command_code) {
            goto command_reading_start;
        }

        command_arg_copy = command_arg;

        //Read command arg once again (it comes after a copy of command code which we already verified)
        //We also verify that there is an > character right after the arg
        //And of course we verify that arg matches the copy we read before.
        do {
            akat_coroutine_state = 5;
akat_coroutine_l_5:

            if (read_arg_and_dequeue() != AKAT_COROUTINE_S_START) {
                return akat_coroutine_state;
            }
        } while (0);

        ;

        if (dequeued_byte != '>' || command_arg_copy != command_arg) {
            goto command_reading_start;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef command_arg
#undef command_arg_copy
#undef command_code
#undef dequeue_byte
#undef dequeued_byte
#undef read_arg_and_dequeue
#undef read_command
}
static AKAT_FORCE_INLINE void usart0_reader() {
#define akat_coroutine_state usart0_reader__akat_coroutine_state
#define command_arg usart0_reader__command_arg
#define command_code usart0_reader__command_code
#define read_command usart0_reader__read_command
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //---- all variable in the thread must be static (green threads requirement)
        ;
        ;

        //Subroutine that reads a command from the input
        //Command is expected to be in the format as one printed out by 'send_status'
        //Command end ups in 'command_code' variable and optional
        //arguments end ups in 'command_arg'. If commands comes without argument, then
        //we assume it is 0 by convention.

//- - - - - - - - - - -
        //Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
        while (1) { //Read command and put results into 'command_code' and 'command_arg'.
            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (read_command() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;

            switch (command_code) {
            case 'C': //TODO: Remove this crap!
                usart0_rx_overflow_count = command_arg;
                break;
            }
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_END;
akat_coroutine_l_end:
    return;
#undef akat_coroutine_state
#undef command_arg
#undef command_code
#undef read_command
}

;







////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Main


static AKAT_FORCE_INLINE void akat_on_every_decisecond() {
    uptime_ticker();
    activity_led();
    ds18b20_ticker();
    ds18b20_aqua__ticker();
    ds18b20_case__ticker();
    co2__ticker();
    co2_ticker();
}

static AKAT_FORCE_INLINE void timer1() {
    OCR1A = 24999;
    TIMSK1 |= (1 << OCIE1A) | (0 << OCIE1B);
    //Configuring Timer1 for prescaler 64
    TCCR1B |= ((0 << CS12) | (1 << CS11) | (1 << CS10) /* Prescaler 64 */) | (1 << WGM12);
}

;




;

// Performs temperature measurements for sensors registered with X_DS18S20$ macro.
// We measure temperature for several sensors at the same time without blocking other threads as much as possible

static u8 ds18b20__tconv_countdown = 0;

;
;


static void ds18b20_ticker() {
//We are waiting for temperature conversion and decrement the counter every 0.1 second
    if (ds18b20__tconv_countdown) {
        ds18b20__tconv_countdown -= AKAT_ONE;
    }
}

;




static u8 ds18b20_thread__akat_coroutine_state = 0;
static u8 ds18b20_thread__byte_to_send = 0;
static u8 ds18b20_thread__command_to_send = 0;
static u8 ds18b20_thread__receive_idx = 0;
static AKAT_FORCE_INLINE u8 ds18b20_thread__has_connected_sensors() {
#define akat_coroutine_state ds18b20_thread__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define receive_idx ds18b20_thread__receive_idx

    if (ds18b20_aqua__connected) {
        return 1;
    }

    if (ds18b20_case__connected) {
        return 1;
    }

    return 0;
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef receive_idx
}
static void ds18b20_thread__write_bit(const u8 bit) {
#define akat_coroutine_state ds18b20_thread__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define receive_idx ds18b20_thread__receive_idx
#define write_bit ds18b20_thread__write_bit

//'bit'can be either zero or non zero. Non zero bit value is treated as 1.

    if (ds18b20_aqua__connected) {
        ds18b20_aqua__pin.set_output_mode();
        ds18b20_aqua__pin.set(0);
    }

    if (ds18b20_case__connected) {
        ds18b20_case__pin.set_output_mode();
        ds18b20_case__pin.set(0);
    }

    if (bit) {//Wait for edge to raise and slave to detect it
        akat_delay_us(4);

        //Set pin to 1 such that slave can sample the value we transmit
        if (ds18b20_aqua__connected) {
            ds18b20_aqua__pin.set_input_mode();
        }

        if (ds18b20_case__connected) {
            ds18b20_case__pin.set_input_mode();
        }
    }

    akat_delay_us(60);

    //Recovery, the pin will be pulled up by the external/internal pull-up resistor
    if (ds18b20_aqua__connected) {
        ds18b20_aqua__pin.set_input_mode();
    }

    if (ds18b20_case__connected) {
        ds18b20_case__pin.set_input_mode();
    }

    akat_delay_us(10);
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef receive_idx
#undef write_bit
}
static void ds18b20_thread__read_bit(u8 mask) {
#define akat_coroutine_state ds18b20_thread__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define read_bit ds18b20_thread__read_bit
#define receive_idx ds18b20_thread__receive_idx
#define write_bit ds18b20_thread__write_bit

//Returns either 0 or non zero (doesn't mean '1'!)

    //Indicate that we want to read a bit
    if (ds18b20_aqua__connected) {
        ds18b20_aqua__pin.set_output_mode();
        ds18b20_aqua__pin.set(0);
    }

    if (ds18b20_case__connected) {
        ds18b20_case__pin.set_output_mode();
        ds18b20_case__pin.set(0);
    }//Allow slave to detect the falling edge on the pin

    akat_delay_us(4);

    //Release the line and let slave set it to the value we will read after the delay
    if (ds18b20_aqua__connected) {
        ds18b20_aqua__pin.set_input_mode();
    }

    if (ds18b20_case__connected) {
        ds18b20_case__pin.set_input_mode();
    }

    akat_delay_us(9);

    if (ds18b20_aqua__connected) {
        if (ds18b20_aqua__pin.is_set()) {
            ds18b20_aqua__received_byte += mask;
        }
    }

    if (ds18b20_case__connected) {
        if (ds18b20_case__pin.is_set()) {
            ds18b20_case__received_byte += mask;
        }
    }//Total duration of reading slot must be at least 60

    akat_delay_us(55);
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef read_bit
#undef receive_idx
#undef write_bit
}
static u8 ds18b20_thread__send_byte__akat_coroutine_state = 0;
static u8 ds18b20_thread__send_byte() {
#define akat_coroutine_state ds18b20_thread__send_byte__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define read_bit ds18b20_thread__read_bit
#define receive_idx ds18b20_thread__receive_idx
#define send_byte ds18b20_thread__send_byte
#define write_bit ds18b20_thread__write_bit
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //LSB (Least significant bit) first order
        write_bit(byte_to_send & H(0));

        do {
            akat_coroutine_state = 2;
            return akat_coroutine_state;
akat_coroutine_l_2:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(1));

        do {
            akat_coroutine_state = 3;
            return akat_coroutine_state;
akat_coroutine_l_3:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(2));

        do {
            akat_coroutine_state = 4;
            return akat_coroutine_state;
akat_coroutine_l_4:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(3));

        do {
            akat_coroutine_state = 5;
            return akat_coroutine_state;
akat_coroutine_l_5:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(4));

        do {
            akat_coroutine_state = 6;
            return akat_coroutine_state;
akat_coroutine_l_6:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(5));

        do {
            akat_coroutine_state = 7;
            return akat_coroutine_state;
akat_coroutine_l_7:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(6));

        do {
            akat_coroutine_state = 8;
            return akat_coroutine_state;
akat_coroutine_l_8:
            ;
        } while (0);

        ;
        write_bit(byte_to_send & H(7));
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef read_bit
#undef receive_idx
#undef send_byte
#undef write_bit
}
static u8 ds18b20_thread__receive_byte__akat_coroutine_state = 0;
static u8 ds18b20_thread__receive_byte() {
#define akat_coroutine_state ds18b20_thread__receive_byte__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define read_bit ds18b20_thread__read_bit
#define receive_byte ds18b20_thread__receive_byte
#define receive_idx ds18b20_thread__receive_idx
#define send_byte ds18b20_thread__send_byte
#define write_bit ds18b20_thread__write_bit
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //LSB (Least significant bit) first order
        ds18b20_aqua__received_byte = 0;
        ds18b20_case__received_byte = 0;
        read_bit(H(0));

        do {
            akat_coroutine_state = 2;
            return akat_coroutine_state;
akat_coroutine_l_2:
            ;
        } while (0);

        ;
        read_bit(H(1));

        do {
            akat_coroutine_state = 3;
            return akat_coroutine_state;
akat_coroutine_l_3:
            ;
        } while (0);

        ;
        read_bit(H(2));

        do {
            akat_coroutine_state = 4;
            return akat_coroutine_state;
akat_coroutine_l_4:
            ;
        } while (0);

        ;
        read_bit(H(3));

        do {
            akat_coroutine_state = 5;
            return akat_coroutine_state;
akat_coroutine_l_5:
            ;
        } while (0);

        ;
        read_bit(H(4));

        do {
            akat_coroutine_state = 6;
            return akat_coroutine_state;
akat_coroutine_l_6:
            ;
        } while (0);

        ;
        read_bit(H(5));

        do {
            akat_coroutine_state = 7;
            return akat_coroutine_state;
akat_coroutine_l_7:
            ;
        } while (0);

        ;
        read_bit(H(6));

        do {
            akat_coroutine_state = 8;
            return akat_coroutine_state;
akat_coroutine_l_8:
            ;
        } while (0);

        ;
        read_bit(H(7));
        ds18b20_aqua__scratchpad[receive_idx] = ds18b20_aqua__received_byte;
        ds18b20_case__scratchpad[receive_idx] = ds18b20_case__received_byte;
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef read_bit
#undef receive_byte
#undef receive_idx
#undef send_byte
#undef write_bit
}
static u8 ds18b20_thread__send_command__akat_coroutine_state = 0;
static u8 ds18b20_thread__send_command() {
#define akat_coroutine_state ds18b20_thread__send_command__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define read_bit ds18b20_thread__read_bit
#define receive_byte ds18b20_thread__receive_byte
#define receive_idx ds18b20_thread__receive_idx
#define send_byte ds18b20_thread__send_byte
#define send_command ds18b20_thread__send_command
#define write_bit ds18b20_thread__write_bit
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //Starts communication:
        //* send reset pulse
        //* wait for presence response
        if (has_connected_sensors()) {//Reset pulse: 480 ... 960 us in low state
            if (ds18b20_aqua__connected) {
                ds18b20_aqua__pin.set_output_mode();
                ds18b20_aqua__pin.set(0);
            }

            if (ds18b20_case__connected) {
                ds18b20_case__pin.set_output_mode();
                ds18b20_case__pin.set(0);
            }

            akat_delay_us(600);

            //Slave awaits 15 ... 60 us
            //and then sinks pin to ground for 60 ... 240 us
            if (ds18b20_aqua__connected) {
                ds18b20_aqua__pin.set_input_mode();
            }

            if (ds18b20_case__connected) {
                ds18b20_case__pin.set_input_mode();
            }

            akat_delay_us(80);

            if (ds18b20_aqua__connected) {
                ds18b20_aqua__connected = !ds18b20_aqua__pin.is_set();

                if (!ds18b20_aqua__connected) {
                    ds18b20_aqua__disconnects += AKAT_ONE;

                    if (!ds18b20_aqua__disconnects) {//We can't go beyond 255
                        ds18b20_aqua__disconnects -= AKAT_ONE;
                    }
                }
            }

            if (ds18b20_case__connected) {
                ds18b20_case__connected = !ds18b20_case__pin.is_set();

                if (!ds18b20_case__connected) {
                    ds18b20_case__disconnects += AKAT_ONE;

                    if (!ds18b20_case__disconnects) {//We can't go beyond 255
                        ds18b20_case__disconnects -= AKAT_ONE;
                    }
                }
            }

            do {
                akat_coroutine_state = 2;
                return akat_coroutine_state;
akat_coroutine_l_2:
                ;
            } while (0);

            ;
            //We must wait for present pulse for minimum of 480 us
            //We have already waited for 80 us in start_initialize + some time in 'yield'
            akat_delay_us(410);

            if (has_connected_sensors()) {
                do {
                    akat_coroutine_state = 3;
                    return akat_coroutine_state;
akat_coroutine_l_3:
                    ;
                } while (0);

                ;
                //Skip ROM
                byte_to_send = 0xCC;

                do {
                    akat_coroutine_state = 4;
akat_coroutine_l_4:

                    if (send_byte() != AKAT_COROUTINE_S_START) {
                        return akat_coroutine_state;
                    }
                } while (0);

                ;

                do {
                    akat_coroutine_state = 5;
                    return akat_coroutine_state;
akat_coroutine_l_5:
                    ;
                } while (0);

                ;
                //Send the command
                byte_to_send = command_to_send;

                do {
                    akat_coroutine_state = 6;
akat_coroutine_l_6:

                    if (send_byte() != AKAT_COROUTINE_S_START) {
                        return akat_coroutine_state;
                    }
                } while (0);

                ;
            }
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_START;
akat_coroutine_l_end:
    return akat_coroutine_state;
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef read_bit
#undef receive_byte
#undef receive_idx
#undef send_byte
#undef send_command
#undef write_bit
}
static AKAT_FORCE_INLINE void ds18b20_thread() {
#define akat_coroutine_state ds18b20_thread__akat_coroutine_state
#define byte_to_send ds18b20_thread__byte_to_send
#define command_to_send ds18b20_thread__command_to_send
#define has_connected_sensors ds18b20_thread__has_connected_sensors
#define read_bit ds18b20_thread__read_bit
#define receive_byte ds18b20_thread__receive_byte
#define receive_idx ds18b20_thread__receive_idx
#define send_byte ds18b20_thread__send_byte
#define send_command ds18b20_thread__send_command
#define write_bit ds18b20_thread__write_bit
    ;
    AKAT_HOT_CODE;

    switch (akat_coroutine_state) {
    case AKAT_COROUTINE_S_START:
        goto akat_coroutine_l_start;

    case AKAT_COROUTINE_S_END:
        goto akat_coroutine_l_end;

    case 2:
        goto akat_coroutine_l_2;

    case 3:
        goto akat_coroutine_l_3;

    case 4:
        goto akat_coroutine_l_4;

    case 5:
        goto akat_coroutine_l_5;

    case 6:
        goto akat_coroutine_l_6;

    case 7:
        goto akat_coroutine_l_7;

    case 8:
        goto akat_coroutine_l_8;

    case 9:
        goto akat_coroutine_l_9;

    case 10:
        goto akat_coroutine_l_10;

    case 11:
        goto akat_coroutine_l_11;

    case 12:
        goto akat_coroutine_l_12;

    case 13:
        goto akat_coroutine_l_13;

    case 14:
        goto akat_coroutine_l_14;

    case 15:
        goto akat_coroutine_l_15;

    case 16:
        goto akat_coroutine_l_16;
    }

akat_coroutine_l_start:
    AKAT_COLD_CODE;

    do {
        //---- All variable in the thread must be static (green threads requirement)
        ;
        ;
        ;

        //---- Functions

//---- Subroutines can yield unlike functions

        //Sends byte from 'byte_to_send', initialization is supposed to be done

//Receive byte into 'received_byte'

//Sends command from 'command_to_send'
        //Does nothing if ds18b20_case__connected is FALSE.
        //Sets ds18b20_case__connected to FALSE if presence pulse is missing!

//- - - - - - - - - - -
        //Main loop in thread (thread will yield on calls to YIELD$ or WAIT_UNTIL$)
        while (1) { //Everything is connected until proven otherwise by presence pulse
            ds18b20_aqua__connected = 1;
            ds18b20_case__connected = 1;
            //Start temperature conversion
            command_to_send = 0x44;

            do {
                akat_coroutine_state = 2;
akat_coroutine_l_2:

                if (send_command() != AKAT_COROUTINE_S_START) {
                    return ;
                }
            } while (0);

            ;

            if (has_connected_sensors()) {//Wait for conversion to end. It takes 750ms to convert, but we "wait"for approx. 900ms ... 1 second
                //tconv_countdown will be decremented every 1/10 second.
                ds18b20__tconv_countdown = 10;

                do {
                    akat_coroutine_state = 3;
akat_coroutine_l_3:

                    if (!(ds18b20__tconv_countdown == 0)) {
                        AKAT_HOT_CODE;
                        return ;
                    }
                } while (0);

                ; //This will YIELD
                //Read scratchpad (temperature)
                command_to_send = 0xBE;

                do {
                    akat_coroutine_state = 4;
akat_coroutine_l_4:

                    if (send_command() != AKAT_COROUTINE_S_START) {
                        return ;
                    }
                } while (0);

                ;

                if (has_connected_sensors()) {
                    receive_idx = 0;

                    do {
                        akat_coroutine_state = 5;
akat_coroutine_l_5:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 1;

                    do {
                        akat_coroutine_state = 6;
akat_coroutine_l_6:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 2;

                    do {
                        akat_coroutine_state = 7;
akat_coroutine_l_7:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 3;

                    do {
                        akat_coroutine_state = 8;
akat_coroutine_l_8:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 4;

                    do {
                        akat_coroutine_state = 9;
akat_coroutine_l_9:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 5;

                    do {
                        akat_coroutine_state = 10;
akat_coroutine_l_10:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 6;

                    do {
                        akat_coroutine_state = 11;
akat_coroutine_l_11:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 7;

                    do {
                        akat_coroutine_state = 12;
akat_coroutine_l_12:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;
                    receive_idx = 8;

                    do {
                        akat_coroutine_state = 13;
akat_coroutine_l_13:

                        if (receive_byte() != AKAT_COROUTINE_S_START) {
                            return ;
                        }
                    } while (0);

                    ;

                    //Check CRC
                    if (ds18b20_aqua__connected) {
                        do {
                            akat_coroutine_state = 14;
                            return ;
akat_coroutine_l_14:
                            ;
                        } while (0);

                        ;
                        //Check CRC
                        u8 crc = akat_crc_add_bytes(0, ds18b20_aqua__scratchpad, 8);

                        if (ds18b20_aqua__scratchpad[8] == crc) {//CRC is OK
                            ds18b20_aqua__updated_deciseconds_ago = 0;
                            ds18b20_aqua__temperatureX16 = ((u16)ds18b20_aqua__scratchpad[1]) * 256 + ds18b20_aqua__scratchpad[0];
                        } else {//CRC is incorrect
                            ds18b20_aqua__crc_errors += AKAT_ONE;

                            if (!ds18b20_aqua__crc_errors) {//We can't go beyond 255
                                ds18b20_aqua__crc_errors -= AKAT_ONE;
                            }
                        }
                    }

                    if (ds18b20_case__connected) {
                        do {
                            akat_coroutine_state = 15;
                            return ;
akat_coroutine_l_15:
                            ;
                        } while (0);

                        ;
                        //Check CRC
                        u8 crc = akat_crc_add_bytes(0, ds18b20_case__scratchpad, 8);

                        if (ds18b20_case__scratchpad[8] == crc) {//CRC is OK
                            ds18b20_case__updated_deciseconds_ago = 0;
                            ds18b20_case__temperatureX16 = ((u16)ds18b20_case__scratchpad[1]) * 256 + ds18b20_case__scratchpad[0];
                        } else {//CRC is incorrect
                            ds18b20_case__crc_errors += AKAT_ONE;

                            if (!ds18b20_case__crc_errors) {//We can't go beyond 255
                                ds18b20_case__crc_errors -= AKAT_ONE;
                            }
                        }
                    }
                }
            }

            do {
                akat_coroutine_state = 16;
                return ;
akat_coroutine_l_16:
                ;
            } while (0);

            ;
        }
    } while (0);

    AKAT_COLD_CODE;
    akat_coroutine_state = AKAT_COROUTINE_S_END;
akat_coroutine_l_end:
    return;
#undef akat_coroutine_state
#undef byte_to_send
#undef command_to_send
#undef has_connected_sensors
#undef read_bit
#undef receive_byte
#undef receive_idx
#undef send_byte
#undef send_command
#undef write_bit
}

;






AKAT_NO_RETURN void main() {
    asm volatile ("EOR r2, r2\nINC r2": "=r"(__akat_one__));
    akat_every_decisecond_run_required = 0;
    G5_unused__init();
    E2_unused__init();
    E3_unused__init();
    E4_unused__init();
    E5_unused__init();
    E6_unused__init();
    E7_unused__init();
    H0_unused__init();
    H1_unused__init();
    H2_unused__init();
    H3_unused__init();
    H4_unused__init();
    H5_unused__init();
    H6_unused__init();
    B0_unused__init();
    B1_unused__init();
    B2_unused__init();
    B3_unused__init();
    B4_unused__init();
    B5_unused__init();
    B6_unused__init();
    H7_unused__init();
    G3_unused__init();
    G4_unused__init();
    L0_unused__init();
    L1_unused__init();
    L2_unused__init();
    L3_unused__init();
    L4_unused__init();
    L5_unused__init();
    L6_unused__init();
    L7_unused__init();
    D0_unused__init();
    D1_unused__init();
    D4_unused__init();
    D5_unused__init();
    D6_unused__init();
    D7_unused__init();
    G0_unused__init();
    G1_unused__init();
    C0_unused__init();
    C1_unused__init();
    C2_unused__init();
    C3_unused__init();
    C4_unused__init();
    C5_unused__init();
    C6_unused__init();
    C7_unused__init();
    J0_unused__init();
    J1_unused__init();
    J2_unused__init();
    J3_unused__init();
    J4_unused__init();
    J5_unused__init();
    J6_unused__init();
    G2_unused__init();
    A7_unused__init();
    A6_unused__init();
    A5_unused__init();
    A4_unused__init();
    A3_unused__init();
    A2_unused__init();
    J7_unused__init();
    K7_unused__init();
    K6_unused__init();
    K5_unused__init();
    K4_unused__init();
    K3_unused__init();
    K2_unused__init();
    K1_unused__init();
    K0_unused__init();
    F7_unused__init();
    F6_unused__init();
    F5_unused__init();
    F4_unused__init();
    F3_unused__init();
    F2_unused__init();
    F1_unused__init();
    F0_unused__init();
    blue_led__init();
    watchdog_init();
    ds18b20_aqua__init();
    ds18b20_case__init();
    co2_init();
    usart0_init();
    timer1();
    //Init
    //Enable interrupts
    sei();

    //Endless loop with threads, tasks and such
    while (1) {
        akat_on_every_decisecond_runner();
        watchdog_reset();
        co2_reader();
        co2_writer();
        usart0_writer();
        usart0_reader();
        ds18b20_thread();
    }
}

;





