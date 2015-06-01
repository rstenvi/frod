include ../../../make_variables.mk
INCLUDE=../../include

C_SOURCES=$(wildcard *.c)
ASM_SOURCES=$(wildcard *.s)

OBJ=${C_SOURCES:.c=.o}
OBJ+=${ASM_SOURCES:.s=.o}

all: $(OBJ)
	ld -N -e start -Ttext 0x7000 -o tmp boot_ap.o
	objcopy -S -O binary -j .text tmp  bootap.bin
	-rm boot_ap.o tmp
	# Usermode program starts at virtual memory 1GB
	ld -N -e start -Ttext 0x40000000 -o tmp usermode.o
	objcopy -S -O binary -j .text tmp  usermode.bin
	-rm usermode.o tmp

clean:
	-rm -f *.o *.bin

.PHONY: all clean
