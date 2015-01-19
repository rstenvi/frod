include ../../make_variables.mk

INCLUDE = "../include/lib/"

C_SOURCES=$(wildcard *.c)
OBJ=${C_SOURCES:.c=.o}

all: $(OBJ)

clean:
	-rm -f *.o

.PHONY: all clean
