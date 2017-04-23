
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

${PROG}: ${MODULES:%=%.o}

%.o: %.c ${HEADERS}

gainer: gainer.c
	@$(CC) -o gainer $(CFLAGS) src/gainer.c $(LDFLAGS)

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}

clean:
	@rm -f ${PROG}-${VER}.tar.gz
	@rm -f ${MODULES:%=%.o}

distclean: clean
	@rm -f ${PROG} server

dist: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist distclean man
