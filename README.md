# ssimCheck
ssimCheck, compare two video files of different dimensions and framelength, output JSON file with frame-by-frame results.

Requires OpenCV for compilation.

## Building:
```
make
```
ssimCheck binary is now in ./bin

```
make install PREFIX=/usr/local
```
ssimCheck binary is now in /usr/local/bin

## Usage:
```
ssimCheck -s <reference video> \
	-t <test video> \
	-o <result file> \
	-n <nth frame> \
	[-a <source frame advance>] \
	[-b <test frame advance>] \
	[-m]
```

### Quick note:
This is something I put together for quality checking a "source" video file against a transcoded version.

Formulas, starting point, and ideas from [here](http://docs.opencv.org/doc/tutorials/highgui/video-input-psnr-ssim/video-input-psnr-ssim.html) and [here](http://en.wikipedia.org/wiki/Peak_signal-to-noise_ratio).

example usage:
```
./ssimCheck -s /a/video/file.mpg -t /a/different/transcoding.ismv -o output.json -n 1 -a 3 -b 5 -m
```

This will check *every frame* (-n 1) in (-s) /a/video/file.mpg against every frame in (-t) /a/different/transcoding.ismv and print a pretty (-o) output.json. 

Before it starts it will offset the source video by (-a) 3 frames from the start, and it will offset the test video by (-b) 5 frames from the start.

Besides the default PSNR values there will also be (-m) RGB SSIM values in the output. Mind you, this makes the program run slower.

-a, -b and -m are optional, as always.
