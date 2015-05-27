CPPFLAGS=$(shell pkg-config --cflags opencv) -std=c++0x
LDFLAGS=$(shell pkg-config --libs opencv)
BINDIR=bin
SRCDIR=src
DESTDIR=
PREFIX=/usr/local

all: ssimCheck

clean:
	rm -f $(BINDIR)/ssimCheck

ssimCheck: $(SRCDIR)/ssimCheck
	cp $(SRCDIR)/ssimCheck $(BINDIR)/ssimCheck

install: ssimCheck
	mkdir -p $(DESTDIR)$(PREFIX)/$(BINDIR)
	cp $(BINDIR)/ssimCheck $(DESTDIR)$(PREFIX)/$(BINDIR)
