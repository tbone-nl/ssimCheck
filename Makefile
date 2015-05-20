INSTALL := $(shell which install)
IFLAGS = -c
CC := $(shell which c++)
CFLAGS := $(shell pkg-config --cflags opencv)
LIBS = /usr/local/lib/libopencv_core.dylib /usr/local/lib/libopencv_videoio.dylib /usr/local/lib/libopencv_video.dylib /usr/local/lib/libopencv_imgproc.dylib
#LIBS := $(shell pkg-config --libs opencv)
DESTDIR = /tmp
BUILDDIR = .
BINDIR = $(BUILDDIR)/bin

ssimCheck: ssimCheck.cpp
	$(CC) $(CFLAGS) $(LIBS) -o $(BINDIR)/ssimCheck ssimCheck.cpp

install: ssimCheck
	$(INSTALL) $(IFLAGS) $(BINDIR)/ssimCheck $(DESTDIR)
