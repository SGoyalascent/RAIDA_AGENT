#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];
unsigned char udp_response[MAXLINE], send_req_buffer[MAXLINE];
unsigned char request_header[REQ_HEAD_MIN_LEN];
int index_req = 0;
time_t t1;


void prepare_send_req_header() {
    
    request_header[REQ_CL] = 0;
    request_header[REQ_SP] = 0;
    request_header[REQ_RI] = raida_id;    // raida_no.
    request_header[REQ_SH] = 0;
    request_header[REQ_CM] = 0;
    request_header[REQ_CM+1] = command_code;   //command code
    request_header[REQ_VE] = 0;
    request_header[REQ_CI] = 0;
    request_header[REQ_CI+1] = 0;
    request_header[REQ_RE] = 0;
    request_header[REQ_RE+1] = 0;
    request_header[REQ_RE+2] = 0;
    request_header[REQ_EC] = 22;
    request_header[REQ_EC+1] = 22;    
    request_header[REQ_FC] = 0;
    request_header[REQ_FC+1] = total_frames;    // udp packets sent
    request_header[REQ_EN] = 0;           //encryption code
    request_header[REQ_ID] = 0;
    request_header[REQ_ID+1] = 0;
    request_header[REQ_SN] = 0;
    request_header[REQ_SN+1] = 0;
    request_header[REQ_SN+2] = 0;

    memcpy(send_req_buffer, request_header, REQ_HEAD_MIN_LEN);
    index_req += REQ_HEAD_MIN_LEN;

    // Add challenge(CH) in the Request Body/buffer
    for(int i=0; i < CH_BYTES_CNT; i++) {
        send_req_buffer[index_req + i] = i+1;
    }

    index_req += CH_BYTES_CNT;

}


void Call_Mirror_Report_Changes() {

    

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