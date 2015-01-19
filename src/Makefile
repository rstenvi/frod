include ../make_variables.mk

PLATFORM=$(wildcard arch/$(ARCH)/*.o)

SYS=$(wildcard sys/*.o)
DRV=$(wildcard drv/*.o)
LIB=$(wildcard lib/*.o)

all: deps kernel

deps:
	make -C ./sys
	make -C ./arch/$(ARCH)
	make -C ./drv
	make -C ./lib

clean:
	make -C ./sys clean
	make -C ./drv clean
	make -C ./arch clean
	make -C ./lib clean
	-rm -f kernel


kernel: deps
	$(LD) $(LDFLAGS) -o kernel $(PLATFORM) $(SYS) $(DRV) $(LIB) $(LDFLAGS2)

.PHONY: all clean deps

