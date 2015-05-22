BINDIR = bin
SRCDIR = src
ssimCheck:
	make -f $(SRCDIR)/Makefile
install: ssimCheck
	cp $(BINDIR)/ssimCheck $(PREFIX)/$(BINDIR)
