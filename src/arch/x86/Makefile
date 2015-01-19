include ../../../make_variables.mk
INCLUDE=../../include

C_SOURCES=$(wildcard *.c)
ASM_SOURCES=$(wildcard *.s)

OBJ=${C_SOURCES:.c=.o}
OBJ+=${ASM_SOURCES:.s=.o}

all: $(OBJ)

clean:
	-rm -f *.o

.PHONY: all clean
