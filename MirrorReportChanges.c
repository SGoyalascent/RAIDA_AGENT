//Get Request from the Primary Raida Agent, 
//Process the Request
//Send the response

#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];
unsigned char udp_response[MAXLINE];
time_t t1;
unsigned int coin_id;
unsigned int table_id;
unsigned int serial_no;
unsigned int index = RES_HS+HS_BYTES_CNT;
unsigned int index_resp = 0;
unsigned int frame_count = 0;
unsigned int frame_no = 0;

union respbody bytes;


void Call_ReportChanges_Mirror() {

    unsigned char request_header[REQUEST_HEADER] = {0,0,2,0,0,45,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0};
    memcpy(send_buffer,request_header,REQUEST_HEADER);

// Add challenge(CH) in the Request Body
    for(int i=0; i < CH_BYTES_CNT; i++) {

        send_buffer[REQUEST_HEADER +i ] = i+1;
    }

    show_dir_content();

//Assign timestamp bytes in the buffer
// YY, MM, DD, HH, MM, SS
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT] = tm.day;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 1] = tm.month;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 2] = tm.year;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 3] = ts.hour;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 4] = ts.minutes;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 5] = ts.second;

//Request body End bytes
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 6] = 62;
    send_buffer[REQUEST_HEADER + CH_BYTES_CNT + 7] = 62;
    int len = REQUEST_HEADER+CH_BYTES_CNT+TIMESTAMP_BYTES_CNT+CMD_END_BYTES_CNT;
    printf("send_buffer:- ");
    for(int i=0; i < len; i++) {

        printf("%d ", buffer[i] );
    }
    printf("\n");

    sendto(sockfd, (const char *)send_buffer, len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

}


void Receive_response_Report_Changes_Mirror() {
    int n;
    n = recvfrom(sockfd, (char *)recv_buffer, MAXLINE, 
	                MSG_WAITALL, (struct sockaddr *) &servaddr,
	                &len);
	printf("n: %d\n", n);

    for(i=0;i<n;i++){	
        printf("%d,", recv_buffer[i]);
    }
    printf("\n");

}


