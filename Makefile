CPP = c++
CPPFLAGS := $(shell pkg-config --cflags opencv)
LDFLAGS := $(shell pkg-config --libs opencv)
BINDIR=bin
SRCDIR=src
DSTDIR=
PREFIX=/usr/local

all: ssimCheck

clean:
	rm -f $(BINDIR)/ssimCheck

ssimCheck: $(SRCDIR)/ssimCheck
	mv $(SRCDIR)/ssimCheck $(BINDIR)/ssimCheck

install: $(BINDIR)/ssimCheck
	cp $(BINDIR)/ssimCheck $(PREFIX)/$(BINDIR)
