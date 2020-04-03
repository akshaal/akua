WRITE_CFLAGS$(firmware);

// NOTE: Sometimes it's nice to try to see which one is best to have some not low, must try some combinations
// USE_REG$(global variable name, low);

USE_REG$(usart0_writer__akat_coroutine_state);
USE_REG$(usart0_writer__byte_to_send);
USE_REG$(usart0_writer__u8_to_format_and_send, low);
USE_REG$(co2_writer__send_byte__akat_coroutine_state, low);
USE_REG$(ds18b20_thread__akat_coroutine_state, low);
USE_REG$(ds18b20_thread__byte_to_send, low);

// TUNE_FUNCTION$(function name, pure, no_inline);
