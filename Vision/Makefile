#
# Custom Makefile for Team 177 VisionServer
# Does not compile depreciated files
#

# Calibration Variables
THRESH=100.0

# Other variables
FLAGS = -O3 `pkg-config --cflags opencv`
PKG_CONFIG_PATH=/usr/local/opencv2.4.8/lib/pkgconfig/
LIBS=`pkg-config --libs opencv`
DEFINES = -DTHRESH=$(THRESH) -DDEBUG -DGUI -DSIM_CAM 
#-DRPI -DGUI

all: VisionServer SendBroadcast

VisionServer: VisionServer.o Receive.o GetImages.o
	g++ VisionServer.o Receive.o GetImages.o $(DEFINES) -o VisionServer -lcurl ${LIBS}

VisionServer.o: VisionServer.cpp VisionServer.h
	g++ -c $(FLAGS) $(DEFINES) VisionServer.cpp -o VisionServer.o

Receive.o: Receive.cpp VisionServer.h
	g++ -c $(FLAGS) $(DEFINES) Receive.cpp -o Receive.o

GetImages.o: GetImages.cpp VisionServer.h
	g++ -c $(FLAGS) $(DEFINES) GetImages.cpp -o GetImages.o
	
SendBroadcast: SendBroadcast.cpp
	g++ SendBroadcast.cpp -o SendBroadcast
	
clean:
	rm -rf *.o  *~ *.stackdump VisionServer.exe VisionServer
	
#fix file formating
indent:
	indent -linux -i4 -nut -l200 *.cpp *.h
	
pi:
	rsync -avz *.cpp *.h pi@10.1.77.91:Vision
	ssh -l pi 10.1.77.91  "cd Vision;make"

