CPP = c++
CPPFLAGS := $(shell pkg-config --cflags opencv)
LDFLAGS := $(shell pkg-config --libs opencv)
DESTDIR = bin

ssimCheck: src/ssimCheck.cpp
	$(CPP) $(CPPFLAGS) $(LDFLAGS) -o $(DESTDIR)/ssimCheck src/ssimCheck.cpp
