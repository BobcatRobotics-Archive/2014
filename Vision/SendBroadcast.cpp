#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

//#define SRV_IP "127.0.0.1"
#define SRV_IP "10.1.77.91"
int main(int argc, char **argv) 
{
    struct sockaddr_in si_other;
    int s, i, slen = sizeof(si_other);
    //char buff[256] = "AUTO";
    //char buff[256] = "TELE";
    int8_t result;
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket");
        exit(1);
    }
     
#ifdef BROADCAST
    int broadcastEnable = 1;
    int ret = setsockopt(s, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    
#endif                          /*  */
        memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(11177);
    
#ifdef BROADCAST	
        si_other.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
#else                           /*  */
        if (inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
        perror("inet_Aton() failed");
        exit(1);
    }
    
#endif                          /*  */
 printf("%s\n", argv[1]);
        if (sendto(s, (char *)argv[1], strlen(argv[1]) + 1, 0, (struct sockaddr *)&si_other, slen) == -1) {
        perror("Sendto()");
        exit(1);
    }
}
