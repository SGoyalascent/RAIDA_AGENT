#include "RAIDA_Agent.h"

unsigned char send_buffer[MAXLINE];

void Call_Mirror_GET_Page() {


    unsigned char request_header[REQUEST_HEADER] = {0,0,2,0,0,45,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0};
    memcpy(send_buffer,request_header,REQUEST_HEADER);

    // Add challenge(CH) in the Request Body
    for(int i=0; i < CH_BYTES_CNT; i++) {

        send_buffer[REQUEST_HEADER +i ] = i+1;
    }

    unsigned int index = REQUEST_HEADER + CH_BYTES_CNT;

    send_buffer[] = ;

}