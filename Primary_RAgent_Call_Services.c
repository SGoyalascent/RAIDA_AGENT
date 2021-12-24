#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];
unsigned char udp_response[MAXLINE], send_req_buffer[MAXLINE];
unsigned char request_header[REQ_HEAD_MIN_LEN];
int index_req = 0;
time_t t1;

//-------------------------------------------------------
//Request Header to Call Mirror Services
//---------------------------------------------------------
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
    request_header[REQ_FC+1] = 1;    // udp packets sent
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
//--------------------------------------------------------
//Send Request to Mirror Raida
//---------------------------------------------------------
void Send_Request_Mirror() {
	sendto(sockfd, (const char *)buffer, len,
	        MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
	            sizeof(servaddr));


}


//-----------------------------------------------------------
//Set time out for UDP frames
//-----------------------------------------------------------
void set_time_out(unsigned char secs){     
	FD_ZERO(&select_fds);            
	FD_SET(sockfd, &select_fds);           	                                  
	timeout.tv_sec = secs; 
	timeout.tv_usec = 0;
}

//-----------------------------------------------------------
//Initialize UDP Socket and bind to the port
//-----------------------------------------------------------
int init_udp_socket() {
	
	printf("init_udp_socket\n");
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	//memset(&cliaddr, 0, sizeof(cliaddr));
	
	servaddr.sin_family = AF_INET; 
	//servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_addr.s_addr = inet_addr(Mirror_agent_config.Ip_address); //Mirror ip address to send request
	servaddr.sin_port = htons(Primary_agent_config.port_number);    //Primary port no.

	if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 ){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
}
//-----------------------------------------------------------
//Receive Response from Mirror Raida
//-----------------------------------------------------------
void Receive_response_Mirror_() {
    unsigned char *buffer,state = STATE_WAIT_START,status_code;
	uint16_t frames_expected=0,curr_frame_no=0,n=0,index=0;
	//uint32_t client_s_addr=0; 	
	socklen_t len = sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);
	while(1){
		//printf("state: %d", state);
		switch(state){
			case STATE_WAIT_START:
				printf("---------------------WAITING FOR RESPONSE HEADER ----------------------\n");	
				memset(buffer,0,server_config_obj.bytes_per_frame);
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &servaddr,&len);
				printf("n: %d\n", n);
				curr_frame_no=1;
				printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
				state = STATE_START_RECVD;	
			break;		
			case STATE_START_RECVD:
				printf("---------------------RESPONSE HEADER RECEIVED ----------------------------\n");
				 status_code = validate_response_header(buffer,n);
				if(status_code != NO_ERR_CODE){
					send_err_resp_header_toMirror(status_code);			
					state = STATE_WAIT_START;
				}else{
					frames_expected = buffer[REQ_FC+1];
					frames_expected|=(((uint16_t)buffer[REQ_FC])<<8); 
					printf("frames expected: %d\n", frames_expected);
					memcpy(udp_buffer,buffer,n);
					index = n;
					client_s_addr = cliaddr.sin_addr.s_addr;
					if(frames_expected == 1){
						state = STATE_END_RECVD;
					}else{
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
						memcpy(&udp_buffer[index],buffer,n);
						index+=n;
						curr_frame_no++;
						printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
						if(curr_frame_no==frames_expected){
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
						printf("---------------------PROCESSING RESPONSE-----------------------------\n");
						process_request(index);
					}
					state = STATE_WAIT_START;
			break;
		}
	}
}

//-----------------------------------------------------------
//  Validate request header
//-----------------------------------------------------------
unsigned char validate_request_header(unsigned char * buff,int packet_size){
	uint16_t frames_expected,request_header_exp_len= REQ_HEAD_MIN_LEN;
	printf("---------------Validate Req Header-----------------\n");
	
	if(buff[REQ_EN]!=0 && buff[REQ_EN]!=1){
		return INVALID_EN_CODE;
	}
	if(packet_size< request_header_exp_len){
		printf("Invalid request header  \n");
		return INVALID_PACKET_LEN;
	}
	frames_expected = buff[REQ_FC+1];
	frames_expected|=(((uint16_t)buff[REQ_FC])<<8);
	if(frames_expected <=0  || frames_expected > FRAMES_MAX){
		printf("Invalid frame count  \n");
		return INVALID_FRAME_CNT;
	}	
	if(buff[REQ_CL]!=0){
		printf("Invalid cloud id \n");
		return INVALID_CLOUD_ID;
	}
	if(buff[REQ_SP]!=0){
		printf("Invalid split id \n");
		return INVALID_SPLIT_ID;
	}
	if(buff[REQ_RI]!=server_config_obj.raida_id){
		printf("Invalid Raida id \n");
		return WRONG_RAIDA;
	}
	return NO_ERR_CODE;
}

//------------------------------------------------------------------------------------------
//  Validate coins and request body and return number of coins 
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body_general(unsigned int packet_len,unsigned int req_body,int *req_header_min){
	*req_header_min = REQ_HEAD_MIN_LEN;
	if(packet_len != (*req_header_min) + req_body){
		send_err_resp_header(INVALID_PACKET_LEN);
		return 0;
	}
	return 1;
}

//-----------------------------------------------------------
// Processes the UDP packet 
//-----------------------------------------------------------
void process_request(unsigned int packet_len){
	uint16_t cmd_no=0, coin_id;
	time_stamp_before = get_time_cs();
	memset(response,0,RESPONSE_HEADER_MAX-1);
	cmd_no = udp_buffer[REQ_CM+1];
	cmd_no |= (((uint16_t)udp_buffer[REQ_CM])<<8);
	coin_id = udp_buffer[REQ_CI+1];
	coin_id |= (((uint16_t)udp_buffer[REQ_CI])<<8);
	switch(cmd_no){
	
		case CMD_COIN_CONVERTER : 			execute_coin_converter(packet_len);break;
		//case CMD_ECHO:						execute_echo(packet_len);break;
		default:							send_err_resp_header(INVALID_CMD);	
	}
}

//-----------------------------------------------------------
// Prepare error response and send it.
//-----------------------------------------------------------
void send_err_resp_header(int status_code){
	int len,size=12;
	unsigned char ex_time;
	char * myfifo = "/tmp/myfifo";
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}

	response[RES_RI] = server_config_obj.raida_id;
	response[RES_SH] = 0;
	response[RES_SS] = status_code;
	response[RES_EX] = 0;
	response[RES_RE] = 0;
	response[RES_RE+1] = 0;
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
	len=sizeof(cliaddr);
	
	sendto(sockfd, (const char *)response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		len);
	
}
//-----------------------------------------------------------
// Prepare response and send it.
//-----------------------------------------------------------
void prepare_resp_header(unsigned char status_code){
	unsigned char ex_time;
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}

	response[RES_RI] = server_config_obj.raida_id;
	response[RES_SH] = 0;
	response[RES_SS] = status_code;
	response[RES_EX] = ex_time;
	response[RES_RE] = 0;
	response[RES_RE+1] = 0;
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;

}







void Call_Mirror_Report_Changes() {

//Assign timestamp bytes in the buffer
// YY, MM, DD, HH, MM, SS
    send_req_buffer[index_req + 0] = tm.year;
    send_req_buffer[index_req + 1] = tm.month;
    send_req_buffer[index_req + 2] = tm.day;
    send_req_buffer[index_req + 3] = tm.hour;
    send_req_buffer[index_req + 4] = tm.minutes;
    send_req_buffer[index_req + 5] = tm.second;

//Request body End bytes
    send_req_buffer[index_req+TIMESTAMP_BYTES_CNT+0] = 62;
    send_req_buffer[index_req+TIMESTAMP_BYTES_CNT+1] = 62;

    int len = index_req + TIMESTAMP_BYTES_CNT + CMD_END_BYTES_CNT;

    printf("send_buffer:- ");
    for(int i=0; i < len; i++) {
        printf("%d ", buffer[i] );
    }
    printf("\n");

    sendto(sockfd, (const char *)send_req_buffer, len, MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

}
