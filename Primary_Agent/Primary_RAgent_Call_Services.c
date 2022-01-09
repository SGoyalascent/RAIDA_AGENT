#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;
unsigned char send_req_buffer[MAXLINE], request_header[REQ_HEAD_MIN_LEN];
unsigned char recv_response[RESPONSE_HEADER_MAX], udp_buffer[UDP_BUFF_SIZE];
unsigned char files_id[FILES_COUNT_MAX][RAIDA_AGENT_FILE_ID_BYTES_CNT], req_file_id[RAIDA_AGENT_FILE_ID_BYTES_CNT];
unsigned int total_files_count = 0;
time_t t1;


//-----------------------------------------------------------
//Initialize UDP Socket
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
	servaddr.sin_port = htons(Mirror_agent_config.port_number);    //Primary port no.

/*
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 ){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	*/
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

//-------------------------------------------------------
//Request Header to Call Mirror Services
//---------------------------------------------------------
int prepare_send_req_header(unsigned char command_code) {
    
	int index_req = 0;

    request_header[REQ_CL] = 0;
    request_header[REQ_SP] = 0;
    request_header[REQ_RI] = server_config_obj.raida_id;    // raida_no.
    request_header[REQ_SH] = 0;
    request_header[REQ_CM] = 0;
    request_header[REQ_CM+1] = command_code;   //command code
    request_header[REQ_VE] = 0;
    request_header[REQ_CI] = 0;
    request_header[REQ_CI+1] = 0;
    request_header[REQ_RE] = 0;
    request_header[REQ_RE+1] = 0;
    request_header[REQ_RE+2] = 0;
    request_header[REQ_EC] = 22;                 //??Test again separetly??//
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

	return index_req;
}

//--------------------------------------------------------
//Send Request to Mirror Raida
//---------------------------------------------------------
void Send_Request(unsigned int size) {
	char * myfifo = "/tmp/myfifo";
	sendto(sockfd, (const char *)send_req_buffer, size,
	        MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));

}

//-----------------------------------------------------------
//Receive Response from Mirror Raida
//-----------------------------------------------------------
int Receive_response() {
    unsigned char *buffer,state = STATE_WAIT_START,status_code;
	unsigned int frames_expected=0,curr_frame_no=0,n=0,index=0;	
	socklen_t len = sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);
	while(1){
		printf("state: %d", state);
		switch(state){
			case STATE_WAIT_START:
				printf("-->SERVICES:-------WAITING FOR RESPONSE-------------\n");	
				memset(buffer,0,server_config_obj.bytes_per_frame);
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &servaddr,&len);
				printf("n: %d\n", n);
				curr_frame_no=1;
				printf("--RECVD_FRAME NO: %d\n", curr_frame_no);
				state = STATE_START_RECVD;	
			break;		
			case STATE_START_RECVD:
				printf("---RESPONSE RECEIVED ------------\n");
				status_code = validate_response_header(buffer,n);
				if(status_code != NO_ERR_CODE){
					printf("Error: Response Header not validated. Error_no: %s\n", status_code);			
					return 0;
				}
				else {
					frames_expected = buffer[RES_RE+1];
					frames_expected|=(((uint16_t)buffer[RES_RE])<<8); 
					//printf("frames expected: %d\n", frames_expected);
					memcpy(recv_response,buffer,n);
					index = n;
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
					printf("ERROR: Fmare Timeout. All frames not received.\n");
					return 0;
				}
				else {
					n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, (struct sockaddr *) &servaddr,&len);
					memcpy(&recv_response[index],buffer,n);
					index += n;
					curr_frame_no++;
					printf("RECVD FRAME NO: %d\n", curr_frame_no);
					if(curr_frame_no==frames_expected){
						state = STATE_END_RECVD;
					}
				}	
			break;			
			case STATE_END_RECVD:
				if(recv_response[index-1] != RESP_END|| recv_response[index-2] != RESP_END){
					printf("Error: Invalid end of Response packet  \n");
					return 0;
				}
				else {
					printf("--------RESPONSE END RECVD---------\n");
				}
			break;
		}
	}
	return index;
}

//-----------------------------------------------------------
//  Validate Response header
//-----------------------------------------------------------
unsigned char validate_response_header(unsigned char * buff,int packet_size){
	int frames_expected,resp_header_exp_len= RESP_HEADER_MIN_LEN;
	printf("---VALIDATE RESPONSE HEADER-------\n");
	
	if(packet_size < resp_header_exp_len){
		printf("ERROR: Invalid Response Header  \n");
		return INVALID_PACKET_LEN;
	}
	frames_expected = buff[RES_RE+1];
	frames_expected|=(((uint16_t)buff[RES_RE])<<8);
	printf("frames_expected: %d\n", frames_expected);
	if(frames_expected <=0  || frames_expected > FRAMES_MAX){
		printf("ERROR: Invalid frame count  \n");
		return INVALID_FRAME_CNT;
	}	
	if(buff[RES_RI] != server_config_obj.raida_id){
		printf("ERROR: Invalid Raida id \n");
		return WRONG_RAIDA;
	}
	if((buff[RES_EC] != send_req_buffer[REQ_EC]) || (buff[RES_EC+1] != send_req_buffer[REQ_EC+1])) {
		printf("Error: EC bytes not match\n");
		return FAIL;
	}
	return NO_ERR_CODE;
}

//------------------------------------------------------------------------------------------
//  Validate Response body 
//-----------------------------------------------------------------------------------------
unsigned char validate_resp_body_report_changes(unsigned int packet_len,int *resp_body,int *resp_header_min){
	*resp_header_min = RESP_HEADER_MIN_LEN;
	*resp_body = packet_len - *resp_header_min - RESP_BODY_END_BYTES;

	printf("Packet_len: %d  Resp_body_without_end_bytes: %d\n", packet_len, *resp_body);
	if((*resp_body != 0) && (*resp_body%RAIDA_AGENT_FILE_ID_BYTES_CNT != 0)) {
		printf("Error: Response body does not match. Invalid Packet length\n");
		return 0;
	}
	if(*resp_body == 0) {
		printf("ERROR: Empty Response Body\n");
		return 0;
	}
	return 1;
}

//----------------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------------

void Call_Report_Changes_Service() {

	printf("-->SERVICES: -------CALL REPORT CHANGES SERVICE---------\n");
	
	unsigned char command_code = MIRROR_REPORT_CHANGES;
	int index_req = prepare_send_req_header(command_code);

	//Assign timestamp bytes in the buffer
	// YY, MM, DD, HH, MM, SS
	printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month+1,tm.year+1900, tm.hour, tm.minutes, tm.second);

    send_req_buffer[index_req + 0] = tm.year;
    send_req_buffer[index_req + 1] = tm.month;
    send_req_buffer[index_req + 2] = tm.day;
    send_req_buffer[index_req + 3] = tm.hour;
    send_req_buffer[index_req + 4] = tm.minutes;
    send_req_buffer[index_req + 5] = tm.second;

	//Request body End bytes
    send_req_buffer[index_req+TIMESTAMP_BYTES_CNT+0] = 62;
    send_req_buffer[index_req+TIMESTAMP_BYTES_CNT+1] = 62;

    unsigned int len = index_req + TIMESTAMP_BYTES_CNT + CMD_END_BYTES_CNT;

    printf("send_buffer:- ");
    for(int i=0; i < len; i++) {
        printf("%d ", buffer[i] );
    }
    printf("\n");

	Send_Request(len);
}

unsigned char Process_response_Report_Changes() {

	unsigned int packet_len = 0, index = 0, size = 0, resp_body_without_end_bytes;
	unsigned char status_code;
	int resp_header_min;

	printf("-->SERVICES: -----PROCESS_RESPONSE_REPORT_CHANGES------\n");

	packet_len = Receive_response();
	if(packet_len == 0) {
		printf("Error: Response received incorrectly. Request Again.\n");
		return FAIL;
	}

	status_code = recv_response[RES_SS];
	printf("STATUS: %d\n", status_code);

	/*
	if(status_code == MIRROR_REPORT_RETURNED) {
		printf("Reuested File names returned\n");
	} 
	else {
		printf("status_code: %s. Error: Status code does not match. Request\n");
		return status_code;
	} */

	if(status_code == RAIDA_AGENT_NO_CHANGES) {
		printf("No Changes. All Files already Synchronized\n");
		return RAIDA_AGENT_NO_CHANGES;
	}
	
	if(validate_resp_body_report_changes(packet_len, &resp_body_without_end_bytes,&resp_header_min) == 0) {
		return FAIL;
	}

	index = resp_header_min;
	total_files_count = resp_body_without_end_bytes/RAIDA_AGENT_FILE_ID_BYTES_CNT;
	printf("No. of files need to be synchronized: %u\n", total_files_count);

	for(int i =0; i < total_files_count; i++) {
		memcpy(&files_id[i][0], &recv_response[index], RAIDA_AGENT_FILE_ID_BYTES_CNT);
		index += RAIDA_AGENT_FILE_ID_BYTES_CNT;
	}

	for(int i=0; i < total_files_count; i++) {
		printf("File_id-%d: ", i+1);
		for(int j=0; j < RAIDA_AGENT_FILE_ID_BYTES_CNT; j++) {
			printf("%d ", files_id[i][j]);
		}
		printf("\n");
	}

	return status_code;
}

void Call_Mirror_Get_Page_Service() {

	printf("------------CALL_MIRROR_GET_PAGE_SERVICE------------------\n");

	unsigned char command_code = AGENT_GET_PAGE;
	int index_req = prepare_send_req_header(command_code);

	memcpy(&send_req_buffer[index_req], &files_id[i][0], RAIDA_AGENT_FILE_ID_BYTES_CNT);
	memcpy(req_file_id, &files_id[i][0], RAIDA_AGENT_FILE_ID_BYTES_CNT);

	send_req_buffer[index_req+0] = 62;
	send_req_buffer[index_req+1] = 62;

	unsigned int len = index_req + RAIDA_AGENT_FILE_ID_BYTES_CNT + CMD_END_BYTES_CNT;

	printf("send_buffer:- ");
    for(int i=0; i < len; i++) {
        printf("%d ", buffer[i] );
    }
    printf("\n");

	Send_Request(len);

}

void Process_response_Get_Page() {

	unsigned int packet_len = 0, index = 0, size = 0;
	unsigned char status_code;

	packet_len = Receive_response();
	status_code = recv_response[RES_SS];

	printf("--------STATUS:");

	if(status_code == SUCCESS) {
		printf("File Page returned\n");
	}
	else if(status_code == MIRROR_REQUESTED_FILE_NOT_EXIST) {
		printf("Requested file does not exist\n");
		return status_code;
	}
	else {
		printf("status_code: %s. Error: Status code does not match\n");
		return status_code;
	}

	index = RESP_HEADER_MIN_LEN;

	unsigned char recv_file_id[RAIDA_AGENT_FILE_ID_BYTES_CNT];

	memcpy(recv_file_id, &recv_response[index], RAIDA_AGENT_FILE_ID_BYTES_CNT);
	index += RAIDA_AGENT_FILE_ID_BYTES_CNT;

	if(memcmp(recv_file_id, req_file_id, RAIDA_AGENT_FILE_ID_BYTES_CNT) != 0) {
		printf("Error: Requested File and Received File not same\n");
		status_code = FAIL;
		return status_code;
	}

	
}