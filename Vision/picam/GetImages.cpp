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
int gmin = 140;
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
	int horiz_cnt = 0;
	int vert_cnt = 0;
    int rc;
    vector < vector < Point > >contours;
    vector < Vec4i > hierarchy;
	bool lhot = false;
	bool rhot = false;
	vector<targetData_t> HRects;
	vector<targetData_t> VRects;
	
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
		RotatedRect rect;
		targetData_t td;
		float ar;		
        for (int i = 0; i < contours.size(); i++) {
            area = contourArea(contours[i]);
			if (area < THRESH) {
#ifdef DEBUG
			cout << i << ": area: "<< area << " - Rejected" << endl;
#endif							
				continue;
			}
			td.area = area;
			td.i = i;
			//td.rect = minAreaRect(contours[i]);
			td.rect = boundingRect(contours[i]);
			contour_cnt++;        
			//Sort by aspect ratio
			ar = (float)td.rect.width/(float)td.rect.height;
#ifdef DEBUG
			cout << i << ": area: "<< area << " ar: " << ar << " " << td.rect.width << "x" << td.rect.height;
#endif			
			if(ar > 2 && ar < 10) {			
				//to simplify the logic below, only record the two biggest in each orientation.
#ifdef DEBUG
				cout << " - Horizontal";
#endif			
				horiz_cnt++;
				if(HRects.size() > 1) {
					int small = 0;
					if(HRects[0].area > HRects[1].area) {
						small = 1;
					}
					if(HRects[small].area < td.area) {
						HRects[small] = td;
					}
				} else {
					HRects.push_back(td);	
				}
			} else if(ar > 0.1 && ar < 0.6) {
#ifdef DEBUG
				cout << " - Vertical";
#endif			
			vert_cnt++;
			if(VRects.size() > 1) {
					int small = 0;
					if(VRects[0].area > VRects[1].area) {
						small = 1;
					}
					if(VRects[small].area < td.area) {
						VRects[small] = td;
					}
				} else {
					VRects.push_back(td);	
				}
			}
#ifdef DEBUG
				cout << endl;
#endif						
        }

		//Determine which side of the goal is hot
		//Can we see both targets?
		if(VRects.size() > 1 && HRects.size() > 0) {
			int h,v;
			int vmin = 0;
			int vmax = 0;
			int hmin = 0;
			int hmax = 0;
			
			//two or more targets - Find the left and right most target in each orientation
			for(v = 1; v < VRects.size(); v++) {
				if(VRects[v].rect.x < VRects[vmin].rect.x) {
					vmin = v;
				}
				if(VRects[v].rect.x > VRects[vmax].rect.x) {
					vmax = v;
				}
			}
			for(h = 1; h < HRects.size(); h++) {
				if(HRects[h].rect.x < HRects[hmin].rect.x) {
					hmin = h;
				}
				if(HRects[h].rect.x > HRects[hmax].rect.x) {
					hmax = h;
				}
			}
			
			if(HRects[hmin].rect.x < VRects[vmin].rect.x) {
				lhot = true;
			}
			
			if(HRects[hmax].rect.x > VRects[vmax].rect.x) {
				rhot = true;
			}
		} else if(VRects.size() == 1 && HRects.size() > 0) {
			//one target
			int htarget = 0;
			if(HRects.size() > 1) {
				//More than one horizontal target, find the closest one
				if(abs(HRects[1].rect.x - VRects[0].rect.x) < abs(HRects[0].rect.x - VRects[0].rect.x)) {
					htarget = 1;
				}
			}
			if(HRects[htarget].rect.x < VRects[0].rect.x) {
				lhot = true;
			}
			
			if(HRects[htarget].rect.x > VRects[0].rect.x) {
				rhot = true;
			}
		}
	}
#ifdef DEBUG
    if(contour_cnt > 0) {
         cout << "Contours found: " << contour_cnt << " Horiz: " << HRects.size() << " Vert: " << VRects.size() << " lhot: " << lhot << " rhot: " << rhot << endl;
    }
#endif	
	pthread_mutex_lock(&data_mutex);
    data.total = contour_cnt;
	data.horiz = HRects.size();
	data.vert = VRects.size();
	data.lhot = lhot;
	data.rhot = rhot;
	// Signal the other thread that there is data available
    pthread_cond_broadcast(&data_ready_cond);
    // Release Mutex
    pthread_mutex_unlock(&data_mutex);
}
