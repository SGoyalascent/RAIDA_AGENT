#include "RAIDA_Agent.h"

union respbody bytes;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];
unsigned char udp_response[MAXLINE];
unsigned int index = RES_HS+HS_BYTES_CNT;
unsigned int frame_no = 0;
unsigned int index_resp = 0;

void ListenRequest_PrimaryAgent() {

	unsigned char *buffer,state=STATE_WAIT_START,status_code;
	uint16_t frames_expected=0,curr_frame_no=0,n=0,i,index=0;
	uint32_t	 client_s_addr=0; 	
	socklen_t len=sizeof(struct sockaddr_in);
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
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL, ( struct sockaddr *) &cliaddr,&len);
				printf("n: %d\n", n);
				curr_frame_no=1;
				printf("--------RECVD  FRAME NO ------ %d\n", curr_frame_no);
				state = STATE_START_RECVD;	
			break;		
			case STATE_START_RECVD:
				printf("---------------------REQ HEADER RECEIVED ----------------------------\n");
				 status_code=validate_request_header(buffer,n);
				if(status_code!=NO_ERR_CODE){
					send_err_resp_header(status_code);			
					state = STATE_WAIT_START;
				}else{
					frames_expected = buffer[REQ_FC+1];
					frames_expected|=(((uint16_t)buffer[REQ_FC])<<8);
					printf("frames_expected: %d\n", frames_expected);
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
					decrypt_request_body(n);
					//print_udp_buffer(n);
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


void execute_Mirror_Get_Page() {


    int req_body_bytes = CH_BYTES_CNT + CMD_END_BYTES_CNT + RAIDA_AGENT_FILES_BYTES_CNT;
	int req_header_min;
	unsigned int index=0;
	unsigned char recv_buffer[RAIDA_AGENT_FILES_BYTES_CNT];
    unsigned int coin_id;
    unsigned int table_id;

	if(validate_request_body_general(packet_len,req_body_bytes,&req_header_min)==0){
		send_err_resp_header(EMPTY_REQ_BODY);
		return;
	}

	index = req_header_min + CH_BYTES_CNT;
	printf("recv_buffer: ");
	for(int i=0; i < RAIDA_AGENT_FILES_BYTES_CNT;i++) {

		recv_buffer[i] = udp_buffer[index+i];
		printf("%d ", recv_buffer[i]);
	}

    bytes.byte_coin[0] = recv_buffer[0];
    bytes.byte_coin[1] = recv_buffer[1];

    coin_id = bytes.val;
    table_id = recv_buffer[2];

    bytes.byte_sn[0] = recv_buffer[3];
    bytes.byte_sn[1] = recv_buffer[4];
    bytes.byte_sn[2] = recv_buffer[5];
    bytes.byte_sn[3] = recv_buffer[6];

    serial_no = bytes.val;

    char root_path[256] = "/opt/Testing/Data";
    char file_path[500];

    strcpy(file_path, root_path);
    //coin_id
    strcat(file_path, "/coin_");
    char c_id[20];
    sprintf(c_id, "%d", coin_id);
    strcat(file_path, c_id);
    //table_id
    if(table_id == 0) {
        strcat(file_path, "/ANs");  
    }
    else if(table_id == 1) {
        strcat(file_path, "/Owners");
    }
    else if(table_id == 2) {
        strcat(file_path, "/Statements");
    }
    else if(table_id == 3) {
        strcat(file_path, "/Loss_Coin_Report");
    }
    else if(table_id == 4) {
        strcat(file_path, "/Email_Recover");
    }
    else {
        printf("table_id: %d\n", table_id);
        printf("Error: Wrong table_id");
    }

    //serial_no

    char sn_no[20];
    sprinf(sn_no, "%d", serial_no);
    strcat(file_path, "/");
    strcat(file_path, sn_no);
    strcat(file_path, ".bin");

    printf("file_path: %s\n", file_path);

}


void Get_File_Contents() {

    FILE *fp_inp = NULL;
    int ch, size = 0;
	char file_path[500];

    fp_inp = fopen(file_path, "rb");
    if(fb_inp == NULL) {
        printf("%d.bin cannot be opened, exiting\n", serial_no);
        return;
    }

    while((ch = fgetc(fp_inp) ) != EOF) {
        size++;
    }
    fclose(fp_inp);

    fp_inp = fopen(file_path, "rb");
    if(fread(response, 1, size, fp_inp) < size) {
        printf("Contents missing in the %d.bin file\n", serial_no);
        return;
    }
    fclose(fp_inp);

    printf("contents: ");
    for(int i=0; i <=size; i++) {
        printf("%d ", response[i]);
    }
    printf("\n");

    index_resp = size;

}

void Send_Response_PrimaryAgent(unsigned char status_code,unsigned int size){
	int len=sizeof(cliaddr);
	char * myfifo = "/tmp/myfifo";
	prepare_mirror_resp_header(status_code);
	sendto(sockfd, (const char *)response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		len);
}

void prepare_mirror_resp_header(unsigned char status_code){
	unsigned char ex_time;
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}

    bytes.val = frame_no;

	udp_response[RES_RI] = server_config_obj.raida_id;
	udp_response[RES_SH] = 0;
	udp_response[RES_SS] = status_code;
	udp_response[RES_EX] = ex_time;
	udp_response[RES_RE] = bytes.byte_coin[0];
	udp_response[RES_RE+1] = bytes.byte_coin[1];
	udp_response[RES_EC] = udp_buffer[REQ_EC];
	udp_response[RES_EC+1] = udp_buffer[REQ_EC+1];
	udp_response[RES_HS] = 0;
	udp_response[RES_HS+1] = 0;
	udp_response[RES_HS+2] = 0;
	udp_response[RES_HS+3] = 0;

}   

void prepare_udp_resp_body() {

    int resp_length = index_resp;
    int current_length = resp_length;
    int total_frames = (resp_length/(MAXLINE-3-index)) + 1;
    int frames = total_frames;
    unsigned char status_code = MIRROR_REPORT_RETURNED;
    int i =0;
    unsigned int size = 0;

	if(resp_length == 0) {
		status_code = MIRROR_REQUESTED_FILE_NOT_EXIST;
		size = RES_HS+HS_BYTES_CNT;
		Send_Response_PrimaryAgent(status_code, size);
		return;
	}

    while(frames >= 1) {

        if(frames > 1) {
            frame_no++;
            memcpy(udp_response[index], response[i], 1008);
            i = i+1008;
            current_length = current_length - 1008;
            udp_response[index+1008+0] = '\n';
            udp_response[index+1008+1] = '\n';
            udp_response[index+1008+2] = '\n';
            size = index+1008+3;
            Send_Response_PrimaryAgent(status_code, size);

        }
        if(frames == 1) {
            frame_no++;
            memcpy(udp_response[index], response[i], current_length);
            udp_response[index+current_length+0] = '\0';
            udp_response[index+current_length+1] = '\0';
            udp_response[index+current_length+2] = '\0';
            size = current_length+index+3;
            Send_Response_PrimaryAgent(status_code, size);
        }
        frames--;
    }


}