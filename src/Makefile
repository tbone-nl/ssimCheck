CPP = c++
CPPFLAGS := $(shell pkg-config --cflags opencv)
LDFLAGS := $(shell pkg-config --libs opencv)
DESTDIR = bin
SRCDIR = src

ssimCheck: $(SRCDIR)/ssimCheck.cpp
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $(DESTDIR)/ssimCheck $(SRCDIR)/ssimCheck.cpp
