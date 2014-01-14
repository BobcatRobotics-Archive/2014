/*
 * Header File
 */

#include <curl/curl.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <string>
#include <cmath>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <pthread.h>
#include <sys/times.h>

//Definitions
#define PI 3.141592653589793238462643383279502884197
#define CURL_TIMEOUT 3000L

//#define DEBUG
#define THRESH 200 
#define RPI
//#define DEBUG
//#define GUI

#ifdef RPI
#define SRV_IP "10.1.77.2"
#else
#define SRV_IP "127.0.0.1"
#endif

//Namespaces
using namespace std;
using namespace cv;

/// Function declarations
void *GetImagesThread(void *arg);
void *ReceiveThread(void *arg);
void *SendThread(void *arg);
void process_frame(cv::Mat frame);

typedef struct {
	uint8_t total;
	uint8_t horiz;
	uint8_t vert;
	uint8_t lhot;
	uint8_t rhot;
} data_t;

typedef struct {
	RotatedRect rect;
	double area;
	uint8_t i;
} targetData_t;

//globals
extern pthread_mutex_t data_mutex;
extern pthread_mutex_t img_mutex;
extern pthread_cond_t data_ready_cond;
extern data_t data;
extern Mat write_img;