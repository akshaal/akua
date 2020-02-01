AVR:

Using Robotdyn ATmega2560 Pro X:
  
  * USB port via CH340C connected to USART0
  * ISP(6pin) header 
  * B7 is used for blue led
  * red power led
  * two other leds is USART0 communication
  
How to test USB/USART/serial comminication:

  LANG=C minicom -D /dev/ttyUSB0 -b 9600
  
Press CTRL-A to pause. Press Esc to unpause.
