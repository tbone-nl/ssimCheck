CPP=/usr/bin/c++

ssimCheck: ssimCheck.cpp
	$(CPP) -I/usr/local/include/opencv -I/usr/local/include /usr/local/lib/libopencv_core.dylib /usr/local/lib/libopencv_videoio.dylib /usr/local/lib/libopencv_video.dylib /usr/local/lib/libopencv_imgproc.dylib -o ssimCheck ssimCheck.cpp

