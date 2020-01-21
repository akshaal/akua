AKAT_DIR=$(PWD)/$({0%/*})/../../../akatlib4

include ${AKAT_DIR}/src/akat.Makefile

MCU=atmega328
SRCS=main.c
TUNING_SRC=tuning.c

PROJ_CFLAGS=-fdump-ipa-inline

all: firmware.avr

distclean: clean

firmware.tmp.c: ${TUNING_SRC} ${SRCS} ${AKAT_SRCS}
		${AKATPP} ${AKATPP_OPTS} ${TUNING_SRC} ${AKAT_SRCS} ${SRCS} > "firmware.tmp.c" || rm "firmware.tmp.c"

firmware.avr: firmware.tmp.c
	  XX=`cat firmware.cflags` && ${CC} -O3 ${CFLAGS} ${PROJ_CFLAGS} $$XX "$<" -save-temps -o $@ && \
		${OBJDUMP} -d $@ > $@.s && \
		${OBJCOPY} -j .text -j .data -O ihex firmware.avr firmware.hex && \
		${NM} --print-size --size-sort --radix=d firmware.avr && \
		${SIZE} $@ && \
		${AKATV} firmware.avr.ltrans0.s && \
		hex2bin firmware.hex && \
		dos2unix firmware.hex

clean:
	rm -f *.ii *.o *.i *.s *.a *.avr *.tmp.c *.hex *.bin
