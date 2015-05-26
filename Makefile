CPP=c++
CPPFLAGS=$(shell pkg-config --cflags opencv)
LDFLAGS=$(shell pkg-config --libs opencv)
BINDIR=bin
SRCDIR=src
DESTDIR=
PREFIX=/usr/local

all: ssimCheck

clean:
	rm -f $(BINDIR)/ssimCheck

ssimCheck: $(SRCDIR)/ssimCheck
	mv $(SRCDIR)/ssimCheck $(BINDIR)/ssimCheck

install: ssimCheck
	cp $(BINDIR)/ssimCheck $(DESTDIR)$(PREFIX)/$(BINDIR)
