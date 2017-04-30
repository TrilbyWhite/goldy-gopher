
CLIENT	=  goldy
SERVER	=  gainer
VER      =  0.1a
CC       ?= gcc
CFLAGS   +=
ifeq ($(TARGET), GPL)
LDLIBS   += -lreadline
CPPFLAGS	+= -DUSE_READLINE
else
LDLIBS   += -ledit
endif
PREFIX   ?= /usr
CPPFLAGS	+= -DPREFIX=\"$(PREFIX)\" -DPROG=\"$(CLIENT)\"
MODULES  =  goldy protocol
HEADERS  =  config.h protocol.h
VPATH    =  src

all: $(CLIENT) $(SERVER)

GPL: all

$(CLIENT): $(MODULES:%=%.o)

%.o: %.c $(HEADERS)

$(SERVER): $(SERVER).c
	@$(CC) -o gainer $(CFLAGS) src/gainer.c -lmagic $(LDFLAGS)

install: $(CLIENT) $(SERVER)
	@install -Dm755 $(CLIENT) $(DESTDIR)$(PREFIX)/bin/$(CLIENT)
	@install -Dm755 $(SERVER) $(DESTDIR)$(PREFIX)/bin/$(SERVER)
	@install -Dm644 -t $(DESTDIR)$(PREFIX)/share/$(CLIENT) share/*
	@sed -i "s|%PREFIX%|$(PREFIX)/share/$(CLIENT)|" $(DESTDIR)$(PREFIX)/share/$(CLIENT)/*
	ifneq ($(TARGET), GPL)
	@install -Dm644 -t $(DESTDIR)$(PREFIX)/share/licenses/$(CLIENT) LICENSE
	endif

clean:
	@rm -f $(CLIENT)-$(VER).tar.gz
	@rm -f $(MODULES:%=%.o)

distclean: clean
	@rm -f $(CLIENT) $(SERVER)

dist: distclean
	@tar -czf $(CLIENT)-$(VER).tar.gz *

.PHONY: clean dist distclean man server GPL
