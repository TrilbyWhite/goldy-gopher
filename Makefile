
CLIENT	=  goldy
SERVER	=  gainer
VER      =  0.1a
#CC       =  musl-clang
DEPS     =  libedit
CFLAGS   += `pkg-config --cflags ${DEPS}` -idirafter /usr/include
LDLIBS   += `pkg-config --libs ${DEPS}`
LDFLAGS  += -L/usr/lib/
PREFIX   ?= /usr
CPPFLAGS	+= -DPREFIX=\"$(PREFIX)\" -DPROG=\"$(CLIENT)\"
MODULES  =  goldy protocol
HEADERS  =  config.h protocol.h
VPATH    =  src

all: $(CLIENT) $(SERVER)

%.o: %.c $(HEADERS)

$(CLIENT): $(MODULES:%=%.o)
	@$(CC) -o goldy goldy.o protocol.o  $(LDLIBS) $(LDFLAGS)

$(SERVER): $(SERVER).c
	@$(CC) -o gainer $(CFLAGS) src/gainer.c -lmagic -lz $(LDFLAGS)

install: $(CLIENT) $(SERVER)
	@install -Dm755 $(CLIENT) $(DESTDIR)$(PREFIX)/bin/$(CLIENT)
	@install -Dm755 $(SERVER) $(DESTDIR)$(PREFIX)/bin/$(SERVER)
	@install -Dm644 -t $(DESTDIR)$(PREFIX)/share/$(CLIENT) share/*
	@sed -i "s|%PREFIX%|$(PREFIX)/share/$(CLIENT)|" $(DESTDIR)$(PREFIX)/share/$(CLIENT)/*

clean:
	@rm -f $(CLIENT)-$(VER).tar.gz
	@rm -f $(MODULES:%=%.o)

distclean: clean
	@rm -f $(CLIENT) $(SERVER)

dist: distclean
	@tar -czf $(CLIENT)-$(VER).tar.gz *

.PHONY: clean dist distclean man server
