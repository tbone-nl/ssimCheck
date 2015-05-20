CC=/usr/bin/c++
CFLAGS=-I/usr/local/include/opencv -I/usr/local/include
LIBS=/usr/local/lib/libopencv_core.dylib /usr/local/lib/libopencv_videoio.dylib /usr/local/lib/libopencv_video.dylib /usr/local/lib/libopencv_imgproc.dylib

ssimCheck: ssimCheck.cpp
	$(CC) $(CFLAGS) $(LIBS) -o ssimCheck ssimCheck.cpp
