
PROG     =  squinny
VER      =  0.1a
CC       ?= gcc
CFLAGS   +=
LDLIBS   += -lreadline
PREFIX   ?= /usr
MODULES  =  squinny protocol
HEADERS  =  protocol.h
MANPAGES =
VPATH    =  src

${PROG}: ${MODULES:%=%.o}

%.o: %.c ${HEADERS}

install: ${PROG}
	@install -Dm755 ${PROG} ${DESTDIR}${PREFIX}/bin/${PROG}

clean:
	@rm -f ${PROG}-${VER}.tar.gz
	@rm -f ${MODULES:%=%.o}

distclean: clean
	@rm -f ${PROG}

dist: distclean
	@tar -czf ${PROG}-${VER}.tar.gz *

.PHONY: clean dist distclean man
