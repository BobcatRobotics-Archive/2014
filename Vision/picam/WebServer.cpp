

#include "VisionServer.h"

void error(const char *msg)
{
    perror(msg);
    pthread_exit(NULL);
}

//save the image to a file to load over the web
void *WebThread(void * arg)
{
    Mat rgbimg;
	int sockfd, newsockfd, portno;
    socklen_t clilen;
    char buffer[2560];   
	struct sockaddr_in serv_addr, cli_addr;
    int n;
    std::vector<unsigned char> imgbuff;
	
	cout << "Starting Web Thread" << endl;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = 80;
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     while (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("ERROR on binding web server to port 80, will retry\n");
		sleep(15);
	 }
     listen(sockfd,5);
	 clilen = sizeof(cli_addr);
	 while(1) 
	 {
		newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
		if (newsockfd < 0) 
			error("ERROR on accept");
		bzero(buffer,2560);
		n = read(newsockfd,buffer,2559);
		if (n < 0) error("ERROR reading from socket");
		//printf("Here is the message: %s\n",buffer);
		if(!strncmp(buffer, "GET", 3)) {
			// Get the next received image
			pthread_mutex_lock(&img_mutex);
			if(!write_img.empty()) {
				write_img.copyTo(rgbimg);
				pthread_mutex_unlock(&img_mutex);
        		imencode(".jpg", rgbimg, imgbuff);
			
				//send the image
				send(newsockfd, &imgbuff[0], imgbuff.size(), 0);
			} else {
				printf("You screwed up!\n");
				pthread_mutex_unlock(&img_mutex);
				send(newsockfd, "No Image", 9, 0);
			}
		} 
		close(newsockfd);
	}
    close(sockfd);
}