#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SRV_IP "127.0.0.1"

{
    
    
    
    
    
        
        
    
    
#ifdef BROADCAST
    int broadcastEnable = 1;
    
    
#endif                          /* 
        
    
    
    
#ifdef BROADCAST	
        si_other.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
#else                           /* 
        if (inet_aton(SRV_IP, &si_other.sin_addr) == 0) {
        
        
    
    
#endif                          /* 
        if (sendto(s, (char *)&buff, strlen(buff) + 1, 0, (struct sockaddr *)&si_other, slen) == -1) {
        
        
    
