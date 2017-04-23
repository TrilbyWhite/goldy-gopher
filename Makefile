
PROG     =  goldy
VER      =  0.1a
CC       ?= gcc
CFLAGS   +=
LDLIBS   += -lreadline
PREFIX   ?= /usr
MODULES  =  goldy protocol
HEADERS  =  config.h protocol.h
MANPAGES =
VPATH    =  src

all: ${PROG} server

${PROG}: ${MODULES:%=%.o}

%.o: %.c ${HEADERS}

server: gainer

gainer: gainer.c
	@$(CC) -o gainer $(CFLAGS) src/gainer.c -lmagic $(LDFLAGS)

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}

clean:
	@rm -f ${PROG}-${VER}.tar.gz
	@rm -f ${MODULES:%=%.o}

distclean: clean
	@rm -f ${PROG} gainer

dist: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist distclean man server
