/*
 * Vision System to Identify FRC Vision Targets Contours
 * Based On: Various OpenCV Sample Code
 * Tuned For: FRC 2014
 * REQUIRES: OpenCV (ARM Compatible)
 * By Daniel Cohen and FRC Team 177
 * Modified for 2014 by D. Schroeder
 */

#include "CVHeader.h"
#include "CurlUtils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#define GUI
#ifdef GUI
#include "opencv2/highgui/highgui.hpp"
Mat dst;
#endif

/// Global Variables
Mat rgbimg; Mat templ; Mat result; Mat dilateimg;
Mat binimg; Mat img; Mat erodeimg; Mat gencanny; Mat canny;
Size dilatesize(20, 20);


// Constants
const int width_in = 24;
const int height_in = 18;


/// Function Headers
void RunServer();
int process();
inline uint64_t endian_swap(uint64_t x);

/**
 * @function main
 */
int main( int argc, char** argv)
{
    cout << "Starting 177 Vision Server" << endl;
	
#ifdef GUI
	// Create window
    namedWindow( "Image", CV_WINDOW_AUTOSIZE );
	
#endif	
	//Initialize the Server 
	RunServer();
}

/**
* @function process
**/
int process() {
  int contour_cnt = 0;
  /// Print diagnostic message
  cout << "Starting frame..." << endl;

  /// Load image from camera (via curl)
  rgbimg = CurlUtils::fetchImg("http://10.1.77.11/axis-cgi/jpg/image.cgi");
  if(rgbimg.empty()) {
	cout << "Empty Image" << endl;
	return -1;
  }

  /// Threshold by BGR values
  inRange(rgbimg, Scalar(160, 0, 0), Scalar(255, 255, 255), binimg);
  
  ///Filter noise in image
  Mat element = getStructuringElement(0, Size(10,10));
  erode(binimg, erodeimg, element);
  Mat dilateelement = getStructuringElement(0, dilatesize);
  dilate(erodeimg, dilateimg, dilateelement);

  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  //Mat img_display;
  //dilateimg.copyTo(img_display);

  /// Detect edges using canny
  Canny( dilateimg, canny_output, 200, 400, 3 );
  /// Find contours
  findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

#ifdef GUI  
  rgbimg.copyTo( dst, canny_output);
  imshow( "Image", dst );
#endif 

  /// Check to see if any targets in the image (Avoids SEGFAULT!)
  if(contours.size() > 0) {

      /// Find contour bigger than threshold with lowest y value
      int miny = canny_output.rows;
      double area = 0;
      RotatedRect temprect;
      RotatedRect minRect;
      for (int i = 0; i < contours.size(); i++) {
        area = contourArea(contours[i]);
        if(area > THRESH) {
		   contour_cnt++;
           /*temprect = minAreaRect(contours[i]);
	   //Make sure we're getting outside contour
	   int midx = temprect.center.x;
           int midy = temprect.center.y-(temprect.size.height/2)+10;
	   if(dilateimg.at<uchar>(midy, midx) > 250) {
             if (temprect.center.y < miny) {
                 miny = temprect.center.y;
                 minRect = temprect;
             }*/
          }
       }
   }

  
  
    cout << "Contours found: " << contour_cnt << endl;
	return contour_cnt;
#if 0	
    /// Check for presence of properly sized contour
	if(contour_cnt == 0) {
	 	cout << "No Target of Sufficient Size!" << endl;
		return 0;
	}

      /// Lots of math to find distance and bearing to target
      double xdist = abs(dilateimg.cols/2 - minRect.center.x); 
      double ydist = abs(dilateimg.rows/2 - minRect.center.y);
      double d = (p0*d0)/minRect.size.height;
      //double m_width_in = (minRect.size.width/minRect.size.height)*height_in;
      //double psi = atan(m_width_in/d);
      //double alpha = asin(d/width_in*sin(psi));
      //double theta = (PI/2) - psi - alpha;
      double deltax = toDegrees(atan(((height_in/minRect.size.height)*xdist)/d));
      double deltay = toDegrees(atan(((height_in*ydist)/minRect.size.height)/d));

      /// Print pertinant results
     /* cout << "W: " << minRect.size.width - dilatesize.width  << endl;
      cout << "H: " << minRect.size.height - dilatesize.height << endl;
      cout << "Center (X, Y): (" << minRect.center.x << ", " << minRect.center.y << ")" << endl;*/

     cout << "Distance: " << d << endl;
     cout << "DeltaX: " << deltax << endl;
     cout << "DeltaY: " << deltay << endl;

     /// Convert to big endian and transmit to cRIO
     uint64_t ud = endian_swap(*(uint64_t *)&d);
     uint64_t udeltax = endian_swap(*(uint64_t *)&deltax);
     uint64_t udeltay = endian_swap(*(uint64_t *)&deltay);

      /// Return pertinant results to Server thread
      //string output = convertNum<double>(d) + "," + convertNum<double>(toDegrees(deltax)) + "," + convertNum<double>(toDegrees(deltay));
      write(sockfd, (char *)&ud, sizeof(ud));
      write(sockfd, (char *)&udeltax, sizeof(udeltax));
      write(sockfd, (char *)&udeltay, sizeof(udeltay));


  } else {
    cout << "Error: No Target in View" << endl;
    return 1;
  }
#endif
}

#define SRV_IP "10.1.77.2"
	
void RunServer() {
	struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buff[256];
	int8_t result;
	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1) {
		perror("socket");
		exit(1);
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(10177);
	if(inet_aton(SRV_IP, &si_other.sin_addr)==0) {
		perror("inet_Aton() failed");
		exit(1);
	}
	
	while(true) {
		result =  process();
		if(sendto(s, (char *)&result, sizeof(int), 0, (struct sockaddr *)&si_other, slen)==-1) {
			perror("Sendto()");
			exit(1);
		}
	}

#if 0
  /// Create server socket and continuously accept connections
  int sockfd, newsockfd, portno = 0;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) {
	  cout << "ERROR: Can't open socket" << endl;
  }
  bzero((char*) &serv_addr, sizeof(serv_addr));
  portno = 10177;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
	  cout << "ERROR: Can't bind" << endl;
  }
  listen(sockfd, 5);
while(true) {
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
  if(newsockfd < 0) {
	cout << "ERROR: Couldn't accept" << endl;
  }

  while(true) {
	int result =  process(P0, D0, newsockfd);
  }
}
#endif
}

/// Converts Radians to Degrees
double toDegrees(double angle)
{
  return angle*(180/PI);
}

/// Swaps endians
inline uint64_t endian_swap(uint64_t x) {
	return (x>>56) |
	    ((x<<40) & 0x00FF000000000000) |
	    ((x<<24) & 0x0000FF0000000000) |
	    ((x<<8)  & 0x000000FF00000000) |
	    ((x>>8)  & 0x00000000FF000000) |
	    ((x>>24) & 0x0000000000FF0000) |
	    ((x>>40) & 0x000000000000FF00) |
            (x<<56);
}


