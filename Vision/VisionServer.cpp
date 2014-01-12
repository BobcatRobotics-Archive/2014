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

// Variables used for thread synchronization
pthread_mutex_t img_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t image_ready_cond = PTHREAD_COND_INITIALIZER;;
uint8_t image_ready = 0;
pthread_t curl_thread, recv_thread;
Mat img_buf;

int process();
int erosion_elem = 0;
int erosion_size = 3;
int dilation_elem = 0;
int dilation_size = 5;
#define RGB
#ifdef RGB
int rmin = 0;
int gmin = 222;
int bmin = 0;
int rmax = 40;
int gmax = 255;
int bmax = 255;
#else
int hmin = 55;
int smin = 45;
int lmin = 36;
int hmax = 95;
int smax = 255;
int lmax = 255;
#endif

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
    process();
    waitKey(10);
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
    cout << "Curl Version: " << curl_version() << endl;
#endif
#ifdef GUI
    // Create window
    namedWindow("Image", CV_WINDOW_AUTOSIZE);
    namedWindow("Controls", CV_WINDOW_AUTOSIZE);
    resizeWindow("Controls", 600, 600);
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

    int rc = pthread_create(&curl_thread, NULL, GetImagesThread, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }

    rc = pthread_create(&recv_thread, NULL, ReceiveThread, NULL);
    if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
    }
    // Initialize the Server 
    RunServer();
}

/**
* @function process
**/
int process()
{
    Mat bgrimg;
    Mat binimg;
    //Mat templ;
    //Mat result;
    Mat dilateimg;
    //Mat img;
    Mat erodeimg;
    int contour_cnt = 0;
    int rc;
    //Mat canny_output;
    vector < vector < Point > >contours;
    vector < Vec4i > hierarchy;
#ifdef DEBUG
    clock_t begin, end;
    int clk_tck = sysconf(_SC_CLK_TCK);
    struct tms t;
    // / Print diagnostic message
    begin = times(&t);
    cout << "Starting frame... " << endl;
#endif

    // Get Mutex
    pthread_mutex_lock(&img_mutex);
    // Wait for data to be available
    rc = 0;
    while (!image_ready && rc == 0) {
        rc = pthread_cond_wait(&image_ready_cond, &img_mutex);
    }
    if (rc != 0) {
        pthread_mutex_unlock(&img_mutex);
        cout << "pthread error: " << rc << endl;
        return -1;
    }
    img_buf.copyTo(bgrimg);
    image_ready = 0;
    // Release Mutex
    pthread_mutex_unlock(&img_mutex);

#ifdef DEBUG
    end = times(&t);
    cout << "Get Time: " << double (end - begin) / clk_tck << endl;
    begin = times(&t);
#endif

    // / Threshold by BGR values
#ifdef RGB
    inRange(bgrimg, Scalar(bmin, gmin, rmin), Scalar(bmax, gmax, rmax), binimg);
#else
    Mat hlsimg;
    //convert to HLS
    cvtColor(bgrimg, hlsimg, CV_BGR2HSV);
    //Threshold
    inRange(hlsimg, Scalar(hmin, smin, lmin), Scalar(hmax, smax, lmax), binimg);
#endif
    // /Filter noise in image
    int erosion_type;
    if (erosion_elem == 0) {
        erosion_type = MORPH_RECT;
    } else if (erosion_elem == 1) {
        erosion_type = MORPH_CROSS;
    } else if (erosion_elem == 2) {
        erosion_type = MORPH_ELLIPSE;
    }
    Mat element = getStructuringElement(erosion_type, Size(2 * erosion_size + 1, 2 * erosion_size + 1),
                                        Point(erosion_size, erosion_size));
    erode(binimg, erodeimg, element);

    int dilation_type;
    if (dilation_elem == 0) {
        dilation_type = MORPH_RECT;
    } else if (dilation_elem == 1) {
        dilation_type = MORPH_CROSS;
    } else if (dilation_elem == 2) {
        dilation_type = MORPH_ELLIPSE;
    }
    Mat dilateelement = getStructuringElement(dilation_type, Size(2 * dilation_size + 1, 2 * dilation_size + 1),
                                              Point(dilation_size, dilation_size));
    dilate(erodeimg, dilateimg, dilateelement);

    // / Find contours
    findContours(dilateimg, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));
#ifdef DEBUG
    end = times(&t);
    cout << "Process Time: " << double (end - begin) / clk_tck << endl;
    begin = times(&t);
#endif
#ifdef GUI
    bgrimg.copyTo(dst);
    if (hierarchy.size() > 0) {
        int idx = 0;
        for (; idx >= 0; idx = hierarchy[idx][0]) {
            Scalar color(rand() & 255, rand() & 255, rand() & 255);
            drawContours(dst, contours, idx, color, CV_FILLED, 80, hierarchy);
        }

    }
    imshow("Image", dst);
#ifdef DEBUG
    end = times(&t);
    cout << "Display Time: " << double (end - begin) / clk_tck << endl;
    begin = times(&t);
#endif
#endif

    // / Check to see if any targets in the image (Avoids SEGFAULT!)
    if (contours.size() > 0) {
        // / Find contour bigger than threshold with lowest y value
        double area = 0;
        for (int i = 0; i < contours.size(); i++) {
            area = contourArea(contours[i]);
#ifdef DEBUG
            cout << i << ": " << area << endl;
#endif
            if (area > THRESH) {
                contour_cnt++;
            }
        }
    }
#ifdef DEBUG
    end = times(&t);
    cout << "Count Time: " << double (end - begin) / clk_tck << endl;
    begin = times(&t);
    cout << "Contours found: " << contour_cnt << endl;
#endif
    return contour_cnt;
}

void RunServer()
{
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    int8_t result;

#ifdef DEBUG
    clock_t begin, end;
    int clk_tck = sysconf(_SC_CLK_TCK);
    struct tms t;
#endif

    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(10177);
    if (inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
        perror("inet_Aton() failed");
        exit(1);
    }

    while (true) {
#ifdef DEBUG
        begin = times(&t);
#endif
        result = process();
        if (sendto(s, (char *)&result, sizeof(int8_t), 0, (struct sockaddr *)&si_other, slen) == -1) {
            perror("Sendto()");
            exit(1);
        }
#ifdef DEBUG
        end = times(&t);
        cout << "** Cycle Time: " << double (end - begin) / clk_tck << endl;
#endif
#ifdef GUI
        if (waitKey(0) == 'q') {
            exit(0);
        }
#endif
    }
}

// / Converts Radians to Degrees
double toDegrees(double angle)
{
    return angle * (180 / PI);
}

// / Swaps endians
inline uint64_t endian_swap(uint64_t x)
{
    return (x >> 56) |
        ((x << 40) & 0x00FF000000000000) |
        ((x << 24) & 0x0000FF0000000000) | ((x << 8) & 0x000000FF00000000) | ((x >> 8) & 0x00000000FF000000) | ((x >> 24) & 0x0000000000FF0000) | ((x >> 40) & 0x000000000000FF00) | (x << 56);
}
