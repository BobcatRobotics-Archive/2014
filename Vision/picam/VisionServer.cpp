/*
 * Vision System to Identify FRC Vision Targets Contours
 * Based On: Various OpenCV Sample Code
 * Tuned For: FRC 2014
 * REQUIRES: OpenCV (ARM Compatible)
 * By Daniel Cohen and FRC Team 177
 * Modified for 2014 by D. Schroeder
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "VisionServer.h"
#include "PiCam.hpp"

extern int n;

// Variables used for thread synchronization
pthread_mutex_t data_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t img_mutex = PTHREAD_MUTEX_INITIALIZER;
Mat write_img;
uint8_t data = 0;
pthread_t send_thread, recv_thread;

extern int erosion_elem;
extern int erosion_size;
extern int dilation_elem;
extern int dilation_size;
#define RGB
#ifdef RGB
extern int rmin;
extern int gmin;
extern int bmin;
extern int rmax;
extern int gmax;
extern int bmax;
#else
int hmin = 55;
int smin = 45;
int lmin = 36;
int hmax = 95;
int smax = 255;
int lmax = 255;
#endif

PiCam *cam;

#ifdef GUI
#include "opencv2/highgui/highgui.hpp"
Mat dst;
int const max_elem = 2;
int const max_kernel_size = 21;

void UpdateGUI(int x, void *p)
{
#ifdef RGB
    cout << "Mins: " << rmin << " " << gmin << " " << bmin << endl;
    cout << "Maxs: " << rmax << " " << gmax << " " << bmax << endl;
#else
    cout << "Mins: " << hmin << " " << smin << " " << lmin << endl;
    cout << "Maxs: " << hmax << " " << smax << " " << lmax << endl;
#endif
    //process();
    //waitKey(10);
}

#endif

// / Function Headers
void RunServer();
int process();
inline uint64_t endian_swap(uint64_t x);

/**
 * @function main
 */
int main(int argc, char **argv)
{
    cout << "Starting 177 Vision Server" << endl;
#ifdef DEBUG
//    cout << "Curl Version: " << curl_version() << endl;
#endif
#ifdef GUI
    // Create window
    namedWindow("Image", CV_WINDOW_AUTOSIZE);
    namedWindow("Controls", CV_WINDOW_AUTOSIZE);
    //resizeWindow("Controls", 600, 600);
    /// Create Erosion Trackbar
    createTrackbar(" E Element:\n 0: Rect \n 1: Cross \n 2: Ellipse", "Controls", &erosion_elem, max_elem, UpdateGUI);

    createTrackbar("E Kernel size:\n 2n +1", "Controls", &erosion_size, max_kernel_size, UpdateGUI);

    /// Create Dilation Trackbar
    createTrackbar("D Element:\n 0: Rect \n 1: Cross \n 2: Ellipse", "Controls", &dilation_elem, max_elem, UpdateGUI);

    createTrackbar("D Kernel size:\n 2n +1", "Controls", &dilation_size, max_kernel_size, UpdateGUI);

#ifdef RGB
    //RGB trackbars
    createTrackbar("R Min", "Controls", &rmin, 255, UpdateGUI);
    createTrackbar("G Min", "Controls", &gmin, 255, UpdateGUI);
    createTrackbar("B Min", "Controls", &bmin, 255, UpdateGUI);
    createTrackbar("R Max", "Controls", &rmax, 255, UpdateGUI);
    createTrackbar("G Max", "Controls", &gmax, 255, UpdateGUI);
    createTrackbar("B Max", "Controls", &bmax, 255, UpdateGUI);
#else
    //HSV trackbars
    createTrackbar("H Min", "Controls", &hmin, 255, UpdateGUI);
    createTrackbar("S Min", "Controls", &smin, 255, UpdateGUI);
    createTrackbar("L Min", "Controls", &lmin, 255, UpdateGUI);
    createTrackbar("H Max", "Controls", &hmax, 255, UpdateGUI);
    createTrackbar("S Max", "Controls", &smax, 255, UpdateGUI);
    createTrackbar("L Max", "Controls", &lmax, 255, UpdateGUI);
#endif
#endif
	int rc;
    rc = pthread_create(&recv_thread, NULL, ReceiveThread, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
	
	rc = pthread_create(&send_thread, NULL, SendThread, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
	
    cam = new PiCam(320, 240, &process_frame);
    //PiCam cam(640, 480, &process_frame);
    while(1) {
       vcos_sleep(10000);
	   std::cout << (n / 10.0) << " FPS\n";
	   n = 0;
    }
}