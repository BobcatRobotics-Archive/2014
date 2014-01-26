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
#ifdef RPI
#define SRV_IP "10.1.77.2"
#else
#define SRV_IP "127.0.0.1"
#endif

//Namespaces
using namespace std;
using namespace cv;

typedef struct {
	//RotatedRect rect;
	Rect rect;
	double area;
	uint8_t i;
} targetData_t;

/// Function declarations
void *GetImagesThread(void *arg);
void *ReceiveThread(void *arg);

//globals
extern pthread_mutex_t img_mutex;
extern pthread_cond_t image_ready_cond;
extern uint8_t image_ready;
extern Mat img_buf;
