
#include "VisionServer.h"
#include <errno.h>
#include "PiCam.hpp"
extern PiCam *cam;

extern pthread_t  picam_thread;
pthread_t record_thread;
bool recording = false;

void *RecordThread(void *arg) {
	recording = true;
	cout << "Starting Record Thread" << endl;
	system("/home/pi/picam/RecordMatch 2>&1 > /media/reclog");
	cout << "Exiting Record Thread" << endl;
	recording = false;
}

//UDP Receive Thread
void *ReceiveThread(void *arg)
{
    struct sockaddr_in si_me, si_in;
    socklen_t slen = sizeof(si_me);
    ssize_t recvcnt;
    int sock_in;
    int broadcast = 1;
    char inbuf[100];
    Mat rgbimg;
    int rc;
    struct timespec ts;
	char fn[256];       
	time_t now;
    struct tm tim;

    cout << "Starting Receive Thread" << endl;

    if ((sock_in = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket in");
        exit(1);
    }
    memset(&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(11177);
    si_me.sin_addr.s_addr = INADDR_ANY;
    assert(::bind(sock_in, (sockaddr *) & si_me, sizeof(sockaddr)) != -1);
    while (1) {
        if ((recvcnt = recvfrom(sock_in, inbuf, sizeof(inbuf) - 1, 0, (sockaddr *) & si_in, &slen)) != 0) {

#ifdef DEBUG
            inbuf[recvcnt] = '\0';
            printf("recv: %s\n", inbuf);

#endif
            if (inbuf[0] == 'A') {
                cout << "Starting Auto" << endl;
				if(recording) {
					system("sudo killall raspivid");
				}
				if(cam == NULL ) {
					//restart the camera if it was stopped
					cout << "Restarting Camera Thread" << endl;
					rc = pthread_create(&picam_thread, NULL, PiCamThread, NULL);
					if (rc) {
						printf("ERROR; return code from pthread_create() is %d\n", rc);
						exit(-1);
					}
				}
                // Store the next received image
                pthread_mutex_lock(&img_mutex);
#if 0
                // Wait for data to be available
                clock_gettime(CLOCK_REALTIME, &ts);
                ts.tv_sec += 5; //wait 5 seconds
                rc = pthread_cond_timedwait(&image_ready_cond, &img_mutex, &ts);
#endif
                if (rc == 0) {
                    write_img.copyTo(rgbimg);
                    // Write the data to a file
                    now = time(NULL);
                    tim = *(localtime(&now));
                    					strftime(fn, 256, "/media/%F-%H%M%S.jpg", &tim);
                    imwrite(fn, rgbimg);
                } else if (rc == ETIMEDOUT) {
                    cout << "Timed out waiting for image in ReceiveThread\n";
                } else {
                    cout << "Thread error in ReceiveThread: " << rc << endl;
                }
                pthread_mutex_unlock(&img_mutex);
			} else if (inbuf[0] == 'S') {
                cout << "Shuting down" << endl;
                system("/usr/bin/sudo shutdown -Ph now");
            } else if (inbuf[0] == 'T') {
                cout << "Starting Teleop" << endl;
                //switch to video recording
				cam->stop();
				delete cam;
				cam = NULL;
				rc = pthread_create(&record_thread, NULL, RecordThread, NULL);
				if (rc) {
					printf("ERROR; return code from pthread_create() is %d\n", rc);
					exit(-1);
				}
            }
        }
    }
}
