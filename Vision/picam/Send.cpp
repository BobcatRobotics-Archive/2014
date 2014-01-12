#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "VisionServer.h"
#include <errno.h>



//UDP Send Thread
void *SendThread(void *arg)
{
	struct sockaddr_in si_other;
	int s, i, slen = sizeof(si_other);
	int rc;
	
	
    cout << "Starting Send Thread" << endl;
    
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
	
	while(1) {	
		rc = 0;
		// Get Mutex
		pthread_mutex_lock(&data_mutex);
		// Wait for data to be available
		rc = pthread_cond_wait(&data_ready_cond, &data_mutex);
		if (rc != 0) {
			pthread_mutex_unlock(&data_mutex);
			cout << "pthread error: " << rc << endl;
			return NULL;
		}
	
		if (sendto(s, (char *)&data, sizeof(int8_t), 0, (struct sockaddr *)&si_other, slen) == -1) {
			perror("Sendto()");
			exit(1);
		}
		
		// Release Mutex
		pthread_mutex_unlock(&data_mutex);
	}
}
