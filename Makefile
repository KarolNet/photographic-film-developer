MCU=atmega8
F_CPU=1000000UL
CC=avr-gcc
OBJCOPY=avr-objcopy
CFLAGS=-Wall -g -Os -mmcu=${MCU} -DF_CPU=${F_CPU} -I.
TARGET=main
SRCS=main.c

all:
	${CC} -mmcu=${MCU} -Wall -gdwarf-2 -std=gnu99 -DF_CPU=${F_CPU} -Os -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -MD -MP -MT ${TARGET} -MF ${TARGET}.o.d  -c  ./main.c
	${CC} -mmcu=${MCU} -Wl,-Map=${TARGET}.map main.o -o ${TARGET}.elf
	${OBJCOPY} -O ihex -R .eeprom -R .fuse -R .lock -R .signature  ${TARGET}.elf ${TARGET}.hex
	${OBJCOPY} -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex ${TARGET}.elf ${TARGET}.eep || exit 0
	avr-objdump -h -S ${TARGET}.elf > ${TARGET}.lss

flash:
	avrdude -p ${MCU} -c usbasp -U flash:w:${TARGET}.hex:i -F -P usb
	avrdude -p ${MCU} -c usbasp -U eeprom:w:${TARGET}.eep:i -F -P usb

clean:
	rm -f *.bin *.hex *.lss *.elf *.o *.map *.eeprom *.o.p
