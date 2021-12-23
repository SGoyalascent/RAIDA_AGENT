#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include "RAIDA_Agent.h"


int listen_request(){
	unsigned char *buffer,state=STATE_WAIT_START,status_code;
	uint16_t frames_expected=0,curr_frame_no=0,n=0,i,index=0,frame_no = 0;
	uint32_t client_s_addr=0; 	
	socklen_t len = sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);
	while(1){
		//printf("state: %d", state);
		switch(state){
			case STATE_WAIT_START:
				printf("---------------------WAITING FOR REQ HEADER ----------------------\n");
				index=0;
				curr_frame_no=0;
				client_s_addr = 0;	
				memset(buffer,0,server_config_obj.bytes_per_frame);
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &cliaddr,&len);
				printf("n: %d\n", n);
				curr_frame_no=1;
				printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
				state = STATE_START_RECVD;	
			break;		
			case STATE_START_RECVD:
				printf("---------------------REQ HEADER RECEIVED ----------------------------\n");
				 status_code = validate_request_header(buffer,n);
				if(status_code != NO_ERR_CODE) {
					send_err_resp_header(status_code);			
					state = STATE_WAIT_START;
				}
				else{
					//udp_packet no = current frame no.
					
					frame_no = buffer[REQ_FC+1];
					frame_no|=(((uint16_t)buffer[REQ_FC])<<8); 
					printf("frame_no: %d\n", frame_no);

					//assuming we are sending header and CMD_END_BYTES each time
					//if only 1 packet is sent
					if((buffer[n-1] == 0x3E) && (buffer[n-2] == 0x3E) && (frame_no == 1)) {
						memcpy(udp_buffer,buffer,n);
						index += n;
						client_s_addr = cliaddr.sin_addr.s_addr;
						state = STATE_END_RECVD;
					}
					//if 1st packet is sent and there are more packets remaining 
					else if((buffer[n-1] == 0x17) && (buffer[n-2] == 0x17)  && (frame_no == 1)) {
						memcpy(udp_buffer,buffer,n-2);
						index += n-2;
						client_s_addr = cliaddr.sin_addr.s_addr;
						state = STATE_WAIT_END;
					}
					
					
					
				}
			break;
			case STATE_WAIT_END:
				set_time_out(FRAME_TIME_OUT_SECS);
				if (select(32, &select_fds, NULL, NULL, &timeout) == 0 ){
					send_err_resp_header(FRAME_TIME_OUT);
					state = STATE_WAIT_START;
					printf("Time out error \n");
				}else{
					n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
					if(client_s_addr==cliaddr.sin_addr.s_addr){
						curr_frame_no++;
						frame_no = buffer[REQ_FC+1];
						frame_no|=(((uint16_t)buffer[REQ_FC])<<8); 
						printf("frame_no: %d\n", frame_no);
						printf("-RECVD  FRAME NO -- %d\n", curr_frame_no);
						//check if both frames equal
						if(curr_frame_no == frame_no) {
							printf("Frame_no matches\n");
						}
						//from 2nd packet onwards
						if((buffer[n-1] == 0x17) && (buffer[n-2] == 0x17)) {
							memcpy(&udp_buffer[index],&buffer[REQ_HEAD_MIN_LEN],n-2-REQ_HEAD_MIN_LEN);
							index += n-2-REQ_HEAD_MIN_LEN;
						}
						//for the last packet
						else if((buffer[n-1] == 0x3E) && (buffer[n-2] == 0x3E)) {
							memcpy(&udp_buffer[index],&buffer[REQ_HEAD_MIN_LEN],n-REQ_HEAD_MIN_LEN);
							index += n-REQ_HEAD_MIN_LEN;
							state = STATE_END_RECVD;
						}
					}						
				}	
			break;			
			case STATE_END_RECVD:
					if(udp_buffer[index-1]!=REQ_END|| udp_buffer[index-2]!=REQ_END){
						send_err_resp_header(INVALID_END_OF_REQ);
						printf("--Invalid end of packet  \n");
					}else{
						printf("---------------------END RECVD----------------------------------------------\n");
						printf("---------------------PROCESSING REQUEST-----------------------------\n");
						process_request(index);
					}
					state = STATE_WAIT_START;
			break;
		}
	}
}
int main() {

	Writr_Binary_File();
    load_server_config();
}