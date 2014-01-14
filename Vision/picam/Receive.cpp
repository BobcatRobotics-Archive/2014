
#include "VisionServer.h"
#include <errno.h>
#include "PiCam.hpp"
extern PiCam *cam;

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
//    setsockopt(sock_in, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast);
//    fcntl(sock_in, F_SETFL, O_NONBLOCK);        // set to non-blocking
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
				if(cam == NULL ) {
					//restart the camera if it was stoped
					cout << "Restarting Camera" << endl;
					cam = new PiCam(320, 240, &process_frame);
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
				cam->Stop();
				delete cam;
				cam = NULL;
				system("/home/pi/picam/RecordMatch 2>&1 > /media/reclog &");
            }
        }
    }
}
