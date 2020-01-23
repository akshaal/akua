AVR:

ATmega328P/CH340G Nano V3:
  
  * USB port via CH340G connected to USART
  * ISP(6pin) header 
  * B5 is used for blue led
  * red power led
  
ATmega328P spec:

  * **Flash:** 32K
  * **EEPROM:** 1K
  * **SRAM:** 2K
  * **10-ADC:** 6
  * **CPU:** 20MHz max
  * **IO-pins:** 23 IO pins
  * **16 bit timer:** 1
  * **8 bit timers:** 2
  * **PWM:** 6
  * **RTC:** 1

How to test USB/USART/serial comminication:

  # Open: LANG=C minicom -D /dev/ttyUSB0
  # Press CTRL-a O
  # Choose "Serial port setup"
  # Select speed 9600
