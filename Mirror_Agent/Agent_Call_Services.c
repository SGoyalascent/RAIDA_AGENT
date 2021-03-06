//Call  Mirror RAIDA Services
//Send Requests to Mirror RIADA and receive Responses from Mirror RIADA

#include "RAIDA_Agent.h"

struct sockaddr_in servaddr;
int sockfd = 0;
fd_set select_fds; 
unsigned char send_req_buffer[MAXLINE], recv_response[RESPONSE_SIZE_MAX];
unsigned char files_id[FILES_COUNT_MAX][RAIDA_AGENT_FILE_ID_BYTES_CNT], req_file_id[RAIDA_AGENT_FILE_ID_BYTES_CNT];
unsigned int total_files_count = 0;
union conversion byteObj;
struct timeval timeout;

//-----------------------------------------------------------
//Initialize UDP Socket
//-----------------------------------------------------------
int init_udp_socket() {
	
	//printf("init_udp_socket\n");
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));	
	servaddr.sin_family = AF_INET; 
	//servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_addr.s_addr = inet_addr((const char*) Mirror_agent_config.Ip_address); //Mirror ip address to send request
	servaddr.sin_port = htons(Primary_agent_config.port_number); //Primary Agent and Mirror Services port no.
	//servaddr.sin_addr.s_addr = inet_addr("172.245.92.198");   
	printf("UDP Socket initialized\n");
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
//Receive Response from Services
//-----------------------------------------------------------
int Receive_response() {
    unsigned char *buffer,state = STATE_WAIT_START,status_code;
	unsigned int frames_expected=0,curr_frame_no=0,n=0,index=0;	
	socklen_t len = sizeof(struct sockaddr_in);
	buffer = (unsigned char *) malloc(server_config_obj.bytes_per_frame);
	
	printf("-------WAITING FOR RESPONSE-------------\n");	
	memset(buffer,0,server_config_obj.bytes_per_frame);
	set_time_out(RESPONSE_TIME_OUT_SECS);
	if (select(32, &select_fds, NULL, NULL, &timeout) == 0 ){
		printf("ERROR: Response Timeout. Response not received.\n");
		return 0;
	}
    n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &servaddr,&len);
    index = n;
    curr_frame_no=1;
    printf("Recvd_Frame_no: %d\n", curr_frame_no);
    status_code = validate_response_header(buffer,n);
    if(status_code != NO_ERR_CODE){
        printf("Error: Response Header not validated. Error_no: %d\n", status_code);			
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
				printf("--------RESPONSE END RECVD---------\n");
				return index;
			break;
		}
	}
	//return index;
}

//-----------------------------------------------------------
//  Validate Response header
//-----------------------------------------------------------
unsigned char validate_response_header(unsigned char * buff,int packet_size){
	int frames_expected,resp_header_exp_len= RESP_HEADER_MIN_LEN;
	//printf("---VALIDATE RESPONSE HEADER-------\n");
	
	if(packet_size < resp_header_exp_len){
		printf("Error: Invalid Response Header  \n");
		return INVALID_PACKET_LEN;
	}
	frames_expected = buff[RES_RE+1];
	frames_expected|=(((uint16_t)buff[RES_RE])<<8);
	printf("frames_expected: %d\n", frames_expected);
	if(frames_expected <= 0  || frames_expected > AGENT_FRAMES_MAX){
		printf("Error: Invalid frame count  \n");
		return INVALID_FRAME_CNT;
	}	
	if(buff[RES_RI] != server_config_obj.raida_id){
		printf("Error: Invalid Raida id \n");
		return WRONG_RAIDA;
	}
	if((buff[RES_EC] != send_req_buffer[REQ_EC]) || (buff[RES_EC+1] != send_req_buffer[REQ_EC+1])) {
		printf("Error: Response echo bytes not match\n");
		return FAIL;
	} 
	//printf("----------RESPONSE HEADER VALIDATED--------------\n");
	return NO_ERR_CODE;
}

//------------------------------------------------------------------------------------------
//  Validate Response body for Report Changes 
//-----------------------------------------------------------------------------------------
unsigned char validate_resp_body_report_changes(unsigned int packet_len,int *resp_body,int *resp_header_min){
	*resp_header_min = RESP_HEADER_MIN_LEN;
	*resp_body = packet_len - *resp_header_min - RESP_BODY_END_BYTES;

	//printf("----VALIDATE RESPONSE BODY----\n");
	printf("Packet_len: %d  Resp_body_without_end_bytes: %d\n", packet_len, *resp_body);
	
	if(*resp_body <= 0) {
		printf("Error: Empty Response Body\n");
		return 0;
	}
	if(recv_response[packet_len-1] != RESP_END|| recv_response[packet_len-2] != RESP_END){
		printf("Error: Invalid end of Response packet\n");
		//printf("resp[1] = %d resp[0] = %d\n",recv_response[packet_len-1], recv_response[packet_len-2]);
		return 0;
	} 
	if((*resp_body%RAIDA_AGENT_FILE_ID_BYTES_CNT != 0)) {
		printf("Error: Response body does not match. Invalid Packet length\n");
		return 0;
	}
	//printf("----RESPONSE BODY VALIDATED----\n");
	return 1;
}
//------------------------------------------------------------------------------------------
//  Validate Response body for GET PAGE Changes 
//-----------------------------------------------------------------------------------------
unsigned char validate_resp_body_get_page(unsigned int packet_len,int *resp_body,int *resp_header_min){
	*resp_header_min = RESP_HEADER_MIN_LEN;
	*resp_body = packet_len - *resp_header_min - RESP_BODY_END_BYTES;

	//printf("----VALIDATE RESPONSE BODY----\n");
	printf("Packet_len: %d  Resp_body_without_end_bytes: %d\n", packet_len, *resp_body);
	if(*resp_body <= 0) {
		printf("ERROR: Empty Response Body\n");
		return 0;
	}
	if(recv_response[packet_len-1] != RESP_END|| recv_response[packet_len-2] != RESP_END){
		printf("Error: Invalid end of Response packet\n");
		//printf("resp[1] = %d resp[0] = %d\n",recv_response[packet_len-1], recv_response[packet_len-2]);
		return 0;
	}
	//printf("----RESPONSE BODY VALIDATED----\n");
	return 1;
}
//--------------------------------------------------------
//Send Request to Mirror Agent Services
//---------------------------------------------------------
void Send_Request(unsigned int size) {
	sendto(sockfd, (const char *)send_req_buffer, size,
	        MSG_CONFIRM, (const struct sockaddr *) &servaddr, sizeof(servaddr));
}
//-------------------------------------------------------
//Request Header to Call Mirror Services
//---------------------------------------------------------
int prepare_send_req_header(unsigned char command_code) {
    
	int index_req = 0;
	unsigned char request_header[REQ_HEAD_MIN_LEN];

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

//----------------------------------------------------------------------------------------
// CALL MIRROR REPORT CHANGES SERVICE || SEND REQUEST TO MIRROR RAIDA SERVICE
//----------------------------------------------------------------------------------------
void Call_Report_Changes_Service() {

	printf("-------CALL MIRROR REPORT CHANGES SERVICE---------\n");
	
	unsigned char command_code = MIRROR_REPORT_CHANGES;
	int index_req, len;
	index_req = prepare_send_req_header(command_code);

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
    len = index_req + TIMESTAMP_BYTES_CNT + CMD_END_BYTES_CNT;
    
	printf("send_buffer:- ");
    for(int i=0; i < len; i++) {
    	printf("%d ", send_req_buffer[i] );
    }
    printf("\n");

	Send_Request(len);
}
//--------------------------------------------------------
//PROCESS REPORT CHANGES RESPONSE
//--------------------------------------------------------
int Process_response_Report_Changes() {

	unsigned int packet_len = 0, index = 0, coin_id, table_id, serial_no;
	unsigned char status_code;
	int resp_header_min, resp_body_without_end_bytes;

	printf("-----PROCESS_RESPONSE_REPORT_CHANGES------\n");
	packet_len = Receive_response();
	if(packet_len == 0) {
		printf("Error: Reveived wrong response. File names not returned\n");
		return 0;
	}
	status_code = recv_response[RES_SS];
	response_status_codes(status_code);

	if(status_code == MIRROR_REPORT_RETURNED) {
		printf("Requested File names returned\n");
	} 
	else if(status_code == RAIDA_AGENT_NO_CHANGES) {
		printf("No Changes. All Files already Synchronized\n");
		return 0;
	}
	else {
		printf("Error: Status code does not match. File names not received\n");
		return 0;
	} 
	if(validate_resp_body_report_changes(packet_len, &resp_body_without_end_bytes,&resp_header_min) == 0) {
		printf("Request body not validated\n");
		return 0;
	}
	/*
	printf("Response: ");
	for(int  i=0; i<packet_len; i++) {
		printf(" %d ", recv_response[i]);
	}
	printf("\n\n"); */

	index = resp_header_min;
	total_files_count = resp_body_without_end_bytes/RAIDA_AGENT_FILE_ID_BYTES_CNT;
	printf("No. of files need to be synchronized: %u\n\n", total_files_count);

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
		/*
		//----------------------------------------------------
		byteObj.byte2[0] = files_id[i][1];  //LSB
		byteObj.byte2[1] = files_id[i][0];   //MSB
		coin_id = byteObj.val;
		table_id = files_id[i][2];
		byteObj.byte4[0] = files_id[i][6];   //LSB
		byteObj.byte4[1] = files_id[i][5];
		byteObj.byte4[2] = files_id[i][4];
		byteObj.byte4[3] = files_id[i][3];   //MSB
		serial_no = byteObj.val;
		printf("coin_id: %d  table_id: %d  serial_no: %d\n", coin_id, table_id, serial_no);

		char id[20], filepath[500];
		strcpy(filepath, execpath);

		if((coin_id == 255) && (table_id == 0)) {
			strcat(filepath, "/my_id_coins/");
			sprintf(id, "%d", serial_no);
			strcat(filepath, id);
			strcat(filepath, ".bin");
		}
		else if((coin_id == 254) && (table_id == 2)) {
			strcat(filepath, "/coin_owners/owners/");
			sprintf(id, "%d", serial_no);
			strcat(filepath, id);
			strcat(filepath, ".bin");
		}
		else if((coin_id == 254) && (table_id == 3)) {
			strcat(filepath, "/coin_owners/statements/");
			sprintf(id, "%d", serial_no);
			strcat(filepath, id);
			strcat(filepath, ".bin");
		}
		else {
			sprintf(id, "%d", coin_id);
			strcat(filepath, "/coin_");
			strcat(filepath, id);
			if(table_id == 1) {
				strcat(filepath, "/ANs/");  
			}
			sprintf(id, "%d", serial_no);
			strcat(filepath, id);
			strcat(filepath, ".bin");
		}
		printf("File_path: %s\n\n", filepath);
		//------------------------------------------------------- */
	}
	return 1;
}
//--------------------------------------------------------
//CALL MIRROR GET PAGE SERVICE
//--------------------------------------------------------
void Call_Mirror_Get_Page_Service(unsigned int i) {

	printf("-----------CALL_MIRROR_GET_PAGE_SERVICE------------------\n");

	unsigned char command_code = AGENT_GET_PAGE;
	int index_req, len;
	index_req = prepare_send_req_header(command_code);

	memcpy(&send_req_buffer[index_req], &files_id[i][0], RAIDA_AGENT_FILE_ID_BYTES_CNT);
	memcpy(req_file_id, &files_id[i][0], RAIDA_AGENT_FILE_ID_BYTES_CNT);
	index_req += RAIDA_AGENT_FILE_ID_BYTES_CNT;
	/*
	for(int j=0; j < RAIDA_AGENT_FILE_ID_BYTES_CNT;j++) {
		printf("files_id[0] = %d  ", files_id[i][j]);
	}
	printf("\n");
	for(int j=0; j < RAIDA_AGENT_FILE_ID_BYTES_CNT;j++) {
		printf("req_file_id = %d\n", req_file_id[j]);
	}
	printf("\n"); */

	send_req_buffer[index_req+0] = REQ_END;
	send_req_buffer[index_req+1] = REQ_END;
	len = index_req + CMD_END_BYTES_CNT;

	printf("req_buffer:- ");
    for(int i=0; i < len; i++) {
        printf("%d ", send_req_buffer[i] );
    }
    printf("\n");

	Send_Request(len);
}
//-----------------------------------------------------------------
//PROCESS GET PAGE RESPONSE
//-----------------------------------------------------------------
int Process_response_Get_Page() {

	unsigned int packet_len = 0, index = 0, file_size, coin_id, table_id, serial_no;
	unsigned char status_code, recv_file_id[RAIDA_AGENT_FILE_ID_BYTES_CNT];;
	int resp_header_min, resp_body_without_end_bytes;
	
	printf("-----PROCESS_RESPONSE_GET_PAGE------\n");

	packet_len = Receive_response();
	if(packet_len == 0) {
		printf("Error: Reveived wrong response. File Page not received\n");
		return 0;
	}
	status_code = recv_response[RES_SS];
	response_status_codes(status_code);
	if(status_code == RAIDA_AGENT_PAGES_RETURNED) {
		printf("File Page returned\n");
	}
	else if(status_code == MIRROR_FILE_NOT_FOUND) {
		printf("Requested file does not found\n");
		return 0;
	}
	else {
		printf("Error: Status code does not match. File Page not returned\n");
		return 0;
	} 
	if(validate_resp_body_get_page(packet_len, &resp_body_without_end_bytes,&resp_header_min) == 0) {
		printf("Request body not validated\n");
		return 0;
	}

	/*printf("Response: ");
	for(int  i=0; i<packet_len; i++) {
		printf(" %d ", recv_response[i]);
	}
	printf("\n\n"); */

	index = resp_header_min;
	memcpy(recv_file_id, &recv_response[index], RAIDA_AGENT_FILE_ID_BYTES_CNT);
	index += RAIDA_AGENT_FILE_ID_BYTES_CNT;

	if(memcmp(recv_file_id, req_file_id, RAIDA_AGENT_FILE_ID_BYTES_CNT) != 0) {
		printf("Error: Requested File and Received File not same\n");
		return 0;
	}
	for(int j=0; j < RAIDA_AGENT_FILE_ID_BYTES_CNT;j++) {
		printf("req_file_id = %d  ", req_file_id[j]);
	}
	printf("\n");
	for(int j=0; j < RAIDA_AGENT_FILE_ID_BYTES_CNT;j++) {
		printf("recv_file_id = %d\n", recv_file_id[j]);
	}
	printf("\n");

	file_size = resp_body_without_end_bytes - RAIDA_AGENT_FILE_ID_BYTES_CNT;

	byteObj.byte2[0] = recv_file_id[1];  //LSB
	byteObj.byte2[1] = recv_file_id[0];   //MSB
	coin_id = byteObj.val;
	table_id = recv_file_id[2];
	byteObj.byte4[0] = recv_file_id[6];   //LSB
	byteObj.byte4[1] = recv_file_id[5];
	byteObj.byte4[2] = recv_file_id[4];
	byteObj.byte4[3] = recv_file_id[3];   //MSB
	serial_no = byteObj.val;

	printf("coin_id: %d  table_id: %d  serial_no: %d\n", coin_id, table_id, serial_no);

	char c_id[20],s_id[20], filepath[500];
	strcpy(filepath, execpath);

	if((coin_id == 255) && (table_id == 0)) {
		strcat(filepath, "/my_id_coins/");
		sprintf(id, "%d", serial_no);
		strcat(filepath, id);
		strcat(filepath, ".bin");
	}
	else if((coin_id == 254) && (table_id == 2)) {
		strcat(filepath, "/coin_owners/owners/");
		sprintf(id, "%d", serial_no);
		strcat(filepath, id);
		strcat(filepath, ".bin");
	}
	else if((coin_id == 254) && (table_id == 3)) {
		strcat(filepath, "/coin_owners/statements/");
		sprintf(id, "%d", serial_no);
		strcat(filepath, id);
		strcat(filepath, ".bin");
	}
	else {
		sprintf(c_id, "%d", coin_id);
		strcat(filepath, "/coin_");
		strcat(filepath, c_id);
		if(table_id == 1) {
			strcat(filepath, "/ANs/");  
		}
		sprintf(s_id, "%d", serial_no);
		strcat(filepath, s_id);
		strcat(filepath, ".bin");
	}
	printf("File_path: %s\n", filepath);
	Update_File_Contents(filepath, file_size, index);

	return 1;
}
//-------------------------------------------------------------------
//UPDATE THE FILES PAGE RECEIVED FROM GET PAGE SERVICE
//------------------------------------------------------------------
void Update_File_Contents(char filepath[], unsigned int file_size, unsigned int index) {

    FILE *fp_inp = NULL;
    char file_path[500];
	strcpy(file_path, filepath);

    fp_inp = fopen(filepath, "wb");
    if(fp_inp == NULL) {
        printf("File cannot be opened, exiting\n");
        return;
    }
    if(fwrite(&recv_response[index], 1, file_size, fp_inp) < file_size) {
        printf("Contents missing in the file\n");
        return;
    }
    fclose(fp_inp);
	printf("File modified and synchronized with the Mirror Raida\n");

}

void response_status_codes(int status_code) {

	switch(status_code) {
		case INVALID_CLOUD_ID:
			printf("STATUS_CODE: %d  STATUS: INVALID_CLOUD_ID\n", INVALID_CLOUD_ID);
		break;
		case RAIDA_OFFLINE:
			printf("STATUS_CODE: %d  STATUS: RAIDA_OFFLINE\n", RAIDA_OFFLINE);
		break;
		case INVALID_FRAME_CNT:
			printf("STATUS_CODE: %d  STATUS: INVALID_UDP_FRAME_CNT\n", INVALID_FRAME_CNT);
		break;
		case INVALID_PACKET_LEN:
			printf("STATUS_CODE: %d  STATUS: INVALID_PACKET_LEN\n", INVALID_PACKET_LEN);
		break;
		case FRAME_TIME_OUT:
			printf("STATUS_CODE: %d  STATUS: FRAME_TIME_OUT\n", FRAME_TIME_OUT);
		break;
		case WRONG_RAIDA:
			printf("STATUS_CODE: %d  STATUS: WRONG_RAIDA_NO\n", WRONG_RAIDA);
		break;
		case INVALID_SPLIT_ID:
			printf("STATUS_CODE: %d  STATUS: INVALID_SPLIT_ID\n", INVALID_SPLIT_ID);
		break;
		case SHARD_NOT_AVAILABLE:
			printf("STATUS_CODE: %d  STATUS: SHARD_NOT_AVAILABLE\n", SHARD_NOT_AVAILABLE);
		break;
		case INVALID_CMD:
			printf("STATUS_CODE: %d  STATUS: INVALID_CMD\n", INVALID_CMD);
		break;
		case INVALID_EN_CODE:
			printf("STATUS_CODE: %d  STATUS: INVALID_ENCRYPTION_CODE\n", INVALID_EN_CODE);
		break;
		case INVALID_END_OF_REQ:
			printf("STATUS_CODE: %d  STATUS: INVALID_END_OF_REQUEST\n", INVALID_END_OF_REQ);
		break;
		case EMPTY_REQ_BODY:
			printf("STATUS_CODE: %d  STATUS: EMPTY_REQUEST_BODY\n", EMPTY_REQ_BODY);
		break;
		case RAIDA_AGENT_PAGES_RETURNED:
			printf("STATUS_CODE: %d  STATUS: RAIDA_AGENT_PAGES_RETURNED\n", RAIDA_AGENT_PAGES_RETURNED);
		break;
		case RAIDA_AGENT_NO_CHANGES:
			printf("STATUS_CODE: %d  STATUS: RAIDA_AGENT_NO_CHANGES_IN_FILES\n", RAIDA_AGENT_NO_CHANGES);
		break;
		case RAIDA_AGENT_PRIMARY_UP:
			printf("STATUS_CODE: %d  STATUS: RAIDA_AGENT_PRIMARY_UP\n", RAIDA_AGENT_PRIMARY_UP);
		break;
		case RAIDA_AGENT_PRIMARY_DOWN:
			printf("STATUS_CODE: %d  STATUS: RAIDA_AGENT_PRIMARY_DOWN\n", RAIDA_AGENT_PRIMARY_DOWN);
		break;
		case MIRROR_MESSAGE_RECEIVED:
			printf("STATUS_CODE: %d  STATUS: MIRROR_MESSAGE_RECEIVED\n", MIRROR_MESSAGE_RECEIVED);
		break;
		case MIRROR_REPORT_RETURNED:
			printf("STATUS_CODE: %d  STATUS: MIRROR_REPORT_RETURNED\n", MIRROR_REPORT_RETURNED);
		break;
		case MIRROR_FILE_NOT_FOUND:
			printf("STATUS_CODE: %d  STATUS: MIRROR_FILE_NOT_FOUND\n", MIRROR_FILE_NOT_FOUND);
		break;
		case SUCCESS:
			printf("STATUS_CODE: %d  STATUS: SUCCESS\n", SUCCESS);
		break;
		case FAIL:
			printf("STATUS_CODE: %d  STATUS: FAIL\n", FAIL);
		break;
		case NO_ERR_CODE:
			printf("STATUS_CODE: %d  STATUS: NO_ERROR_CODE\n", NO_ERR_CODE);
		break;
	}
}