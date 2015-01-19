
C_SOURCES=$(wildcard *.c)
OBJ=${C_SOURCES:.c=.o}

INCLUDE=../include
include ../../make_variables.mk

all: $(OBJ)

clean:
	-rm -f *.o

.PHONY: all clean
