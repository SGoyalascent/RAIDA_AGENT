#include "RAIDA_Agent.h"


int Receive_response() {
    unsigned char *buffer,state,status_code;
	unsigned int frames_expected=0,curr_frame_no=0,n=0,index=0;	
	socklen_t len = sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);

    printf("-->SERVICES:-------WAITING FOR RESPONSE-------------\n");	
    memset(buffer,0,server_config_obj.bytes_per_frame);
    n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &servaddr,&len);
    index = n;
    curr_frame_no=1;
    printf("Recvd_Frame_no: %d\n", curr_frame_no);
    state = STATE_START_RECVD;	
    status_code = validate_response_header(buffer,n);
    if(status_code != NO_ERR_CODE){
        printf("Error: Response Header not validated. Error_no: %s\n", status_code);			
        return 0;
    }
    memcpy(recv_response,buffer,n);
    frames_expected = buffer[RES_RE+1];
	frames_expected|=(((uint16_t)buffer[RES_RE])<<8); 
    if(frames_expected == 1){
		state = STATE_END_RECVD;
	}
    else{
        state = STATE_WAIT_END;
    }
	while(1){
		//printf("state: %d", state);
		switch(state){	
			case STATE_WAIT_END:
				set_time_out(FRAME_TIME_OUT_SECS);
				if (select(32, &select_fds, NULL, NULL, &timeout) == 0 ){
					printf("ERROR: Frame Timeout. All frames not received.\n");
					return 0;
				}
				else {
					n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, (struct sockaddr *) &servaddr,&len);
					memcpy(&recv_response[index],buffer,n);
					index += n;
					curr_frame_no++;
					printf("Recvd_Frame_no: %d\n", curr_frame_no);
					if(curr_frame_no==frames_expected){
						state = STATE_END_RECVD;
					}
				}	
			break;			
			case STATE_END_RECVD:
				if(recv_response[index-1] != RESP_END|| recv_response[index-2] != RESP_END){
					printf("Error: Invalid end of Response packet\n");
					return 0;
				}
				else {
					printf("--------RESPONSE END RECVD---------\n");
                    return index;
				}
			break;
		}
	}
	return index;
}