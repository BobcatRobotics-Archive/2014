#
# Custom Makefile for Team 177 VisionServer
# Does not compile depreciated files
#

# Calibration Variables
P0=63
D0=115
THRESH=6000.0

# Other variables
FLAGS = -O3 `pkg-config --cflags opencv`
PKG_CONFIG_PATH=/usr/local/opencv2.4.8/lib/pkgconfig/
LIBS=`pkg-config --libs opencv`

all: visionserver

visionserver: visionserver.o curlutils.o
	g++ visionserver.o curlutils.o -o VisionServer -lcurl ${LIBS}

visionserver.o: VisionServer.cpp
	g++ -c $(FLAGS) -DP0=$(P0) -DD0=$(D0) -DTHRESH=$(THRESH) VisionServer.cpp -o visionserver.o

curlutils.o: CurlUtils.cpp
	g++ -c $(FLAGS) CurlUtils.cpp -o curlutils.o

clean:
	rm -rf *.o VisionServer
