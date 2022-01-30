// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

 #define VER 255
#define PORT     18000
#define MAXLINE 1024
#define ENCRYPTION_CONFIG_BYTES  16
#define NOUNCE_BYTES_CNT         8
#define FRAME_TIME_OUT_SECS		1

#define REQ_NO_1  				9
#define REQ_NO_2  				10
#define REQ_NO_3  				11
#define REQ_NO_4  				12
#define REQ_NO_5  				13

#define REQ_NO_6				19
#define REQ_NO_7				20
#define REQ_NO_8				21
  //2021-07-15 17:15:15
// Driver code

char execpath[256];
uint8_t encrypt_key[ENCRYPTION_CONFIG_BYTES];
uint8_t nounce[NOUNCE_BYTES_CNT];
struct timeval timeout;
fd_set select_fds; 
int sockfd;
	

int main() {
    int i = 0;
    char *token = "139.99.155.124";
    char* Ip_address;
    unsigned int port_number;
    unsigned char buffer[MAXLINE], recv_buffer[MAXLINE];	
    unsigned char buffer_version[MAXLINE]={0,0,2,0,0,15,2,0,0,0,0,0,22,22,0,1,1,0,0,0,0,0,0x3E,0x3E};
    unsigned char buffer_echo[MAXLINE]={0,0,2,0,0,4,2,0,0,0,0,0,22,22,0,1,1,0,0,0,0,0,0x3E,0x3E};


    strcpy(Ip_address, token);

	struct sockaddr_in     servaddr;
			
	//  memcpy(buffer,buffer_version,MAXLINE);			
		memcpy(buffer,buffer_echo,MAXLINE);	

	    // Creating socket file descriptor
	    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	        perror("socket creation failed");
	        exit(EXIT_FAILURE);
	    }
		else {
			printf("Socket Creation Successful\n");
		}
	  
	    memset(&servaddr, 0, sizeof(servaddr));
	    servaddr.sin_family = AF_INET;
	    servaddr.sin_port = htons(port_number);
	   servaddr.sin_addr.s_addr = inet_addr((const char*) Ip_address);

	      
	    int n, len=0;
	     for(i=0;i<MAXLINE;i++){
		len++;
		//printf("%d,",buffer[i]);
		if(buffer[i]==62 && buffer[i-1]==62)
			break;	
	    }
	   printf("Len = %d %d\n", len,i);

	printf("buffer: ");
	for(int i=0; i < len; i++) {
		printf("%d  ", buffer[i]);
	}
	printf("\n");

	    sendto(sockfd, (const char *)buffer, len,
	        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
	            sizeof(servaddr));
	    n = recvfrom(sockfd, (char *)recv_buffer, MAXLINE, 
	                MSG_WAITALL, (struct sockaddr *) &servaddr,
	                &len); 

		printf("n: %d\n", n);
	   for(i=0;i<n;i++){	
	     printf("%d,", recv_buffer[i]);
	   }
	   printf("\n");

    
	close(sockfd);

  	return 0;
}