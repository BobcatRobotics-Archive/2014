#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "VisionServer.h"
#include "PiCam.hpp"
int n = 0;
int erosion_elem = 0;
int erosion_size = 1;
int dilation_elem = 0;
int dilation_size = 2;
#define RGB
#ifdef RGB
int rmin = 0;
int gmin = 200;
int bmin = 0;
int rmax = 255;
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

extern int s, slen;
extern struct sockaddr_in si_other;
	
void process_frame(cv::Mat frame) {
    Mat bgrimg;
    Mat binimg;
    Mat dilateimg;
    Mat erodeimg;
    cv::Mat_<unsigned char> dst(frame.size());
	int contour_cnt = 0;
    int rc;
    vector < vector < Point > >contours;
    vector < Vec4i > hierarchy;
	
	n++;
	 // / Threshold by BGR values
#ifdef RGB
	cvtColor(frame, bgrimg, CV_RGB2BGR);
	pthread_mutex_lock(&img_mutex);
	bgrimg.copyTo(write_img);
	pthread_mutex_unlock(&img_mutex);
    inRange(bgrimg, Scalar(bmin, gmin, rmin), Scalar(bmax, gmax, rmax), binimg);
#else
    Mat hlsimg;
    //convert to HLS
    cvtColor(frame, hlsimg, CV_BGR2HSV);
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
	waitKey(1);
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
    if(contour_cnt > 0) {
        cout << "Contours found: " << contour_cnt << endl;
    }
#endif	
	pthread_mutex_lock(&data_mutex);
    data = contour_cnt;
	// Signal the other thread that there is data available
    pthread_cond_broadcast(&data_ready_cond);
    // Release Mutex
    pthread_mutex_unlock(&data_mutex);
}
