include ../../make_variables.mk

C_SOURCES=$(wildcard *.c)
OBJ=${C_SOURCES:.c=.o}

INCLUDE=../include

all: $(OBJ)

clean:
	-rm -f *.o

.PHONY: all clean
