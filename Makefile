
PROG     =  gopher
CC       ?= gcc
LDLIBS   += -lreadline
PREFIX   ?= /usr

all: ${PROG}

clean:
	@rm ${PROG}

.PHONY: clean
