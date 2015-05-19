# ssimCheck
ssimCheck, compare two video files of different dimensions and framelength, output JSON file with frame-by-frame results.
Requires OpenCV for compilation.

## Building:
```
cmake .
make
./ssimCheck ...
```
... or alternatively use the "works on my machine" Makefile:
```
make ssimCheck
./ssimCheck ...
```

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
