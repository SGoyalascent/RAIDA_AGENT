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




//-----------------------------------------------------------
//Initialize UDP Socket and bind to the port
//-----------------------------------------------------------
int init_udp_socket(){}

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
//-----------------------------------------------------------
//  Validate request header
//-----------------------------------------------------------
unsigned char validate_request_header(unsigned char * buff,int packet_size){
	uint16_t frames_expected,i=0,request_header_exp_len= REQ_HEAD_MIN_LEN, coin_id=0;
	printf("---------------Validate Req Header-----------------\n");
	
	if(buff[REQ_EN]!=0 && buff[REQ_EN]!=1){
		return INVALID_EN_CODE;
	}
	request_header_exp_len = REQ_HEAD_MIN_LEN;
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

unsigned char validate_request_body_general(unsigned int packet_len,unsigned int req_body,int *req_header_min){
	*req_header_min = REQ_HEAD_MIN_LEN;
	if(packet_len != (*req_header_min) + req_body){
		send_err_resp_header(INVALID_PACKET_LEN);
		return 0;
	}
	return 1;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------



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
		status_code = RAIDA_AGENT_NO_CHANGES;
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
            udp_response[index+1008+0] = 0x17;
            udp_response[index+1008+1] = '\n';
            udp_response[index+1008+2] = 0x17;
            size = index+1008+3;
            Send_Response_PrimaryAgent(status_code, size);

        }
        if(frames == 1) {
            frame_no++;
            memcpy(udp_response[index], response[i], current_length);
            udp_response[index+current_length+0] = 0x3E;
            udp_response[index+current_length+1] = '\0';
            udp_response[index+current_length+2] = 0x3E;
            size = current_length+index+3;
            Send_Response_PrimaryAgent(status_code, size);
        }
        frames--;
    }


}

int prepare_resp_body(int index_resp) {
    
    bytes.val = coin_id;

    response[index_resp+0] = bytes.byte_coin[0];
    response[index_resp+1] = bytes.byte_coin[1];
    response[index_resp+2] = table_id;

    bytes.val = serial_no;

    response[index_resp+3] = bytes.byte_sn[0];
    response[index_resp+4] = bytes.byte_sn[1];
    response[index_resp+5] = bytes.byte_sn[2];
    response[index_resp+6] = bytes.byte_sn[3];
    
    return (index_resp+7);
}

void get_ModifiedFiles(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    
	char root_path[256] = "/opt/Testing/Data";
    int path_len = strlen(root_path);

    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char df_path[500];
        char df_name[256];
        sprintf(df_name, "%s",dir->d_name);
        sprintf(df_path, "%s/%s", path, dir->d_name);

        if(dir->d_type == DT_REG) {
            
			printf("filename: %s  filepath: %s\n", df_name, df_path);

            //Compare timestamps
            time_t t2;
            char datestring[256];
            double time_dif;
            char sub_path[500];

            if(stat(df_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }

            strcpy(sub_path, df_path+path_len+1);
            printf("sub_path: %s\n", sub_path);

            if(strcmp(sub_path, df_name) == 0) {
                continue;
            }

            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s\n", datestring);
            printf("Last Modified Time(UTC):- %d-%d-%d  %d:%d:%d\n", dt->tm_mday,dt->tm_mon+1,dt->tm_year+1900, 
                                                                                dt->tm_hour,dt->tm_min, dt->tm_sec );
        
            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif <= 0) {
                printf("File already Syncronized.\n");
                continue;
            }
            printf("File Modified. Need to be Syncronized.\n");
            
            char coin[20];  //coin_1234
            char table[20];  //Statements
            unsigned char c_id[10];  //1234 from coin[]

            char *token;
            token = strtok(sub_path, "/");
            strcpy(coin, token);

            token = strtok(NULL, "/");
            strcpy(table, token);

            token = strtok(coin, "_");
            token = strtok(NULL,"_");
            strcpy(c_id, token);
            coin_id = atoi(c_id);
            serial_no = atoi(df_name);   //12345.bin file -->  12345

            if(strcmp(table, "ANs") == 0) {
                table_id = 0;
            }
            else if(strcmp(table, "Owners") == 0) {
                table_id = 1;
            }
            else if(strcmp(table, "Statements") == 0) {
                table_id = 2;
            }
            else if(strcmp(table, "Loss_Coin_Report") == 0) {
                table_id = 3;
            }
            else if(strcmp(table, "Email_Recover") == 0) {
                table_id = 4;
            }
        
            printf("coin_id: %d  table_id: %d  serial_no: %d\n", coin_id, table_id, serial_no);

			index = prepare_resp_body(index);
        }	
			
        if((dir->d_type == DT_DIR) && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("dir_name: %s  dir_path: %s\n", df_name,df_path);
            get_ModifiedFiles(df_path); 
        }
    }
    closedir(d);
}


void  execute_Report_Changes(unsigned int packet_len) {

	int req_body_bytes = CH_BYTES_CNT + CMD_END_BYTES_CNT + TIMESTAMP_BYTES_CNT;
	int req_header_min;
	unsigned int index=0;
	unsigned char recv_buffer[TIMESTAMP_BYTES_CNT];

	if(validate_request_body_general(packet_len,req_body_bytes,&req_header_min)==0){
		send_err_resp_header(EMPTY_REQ_BODY);
		return;
	}

	index = req_header_min + CH_BYTES_CNT;
	printf("recv_buffer: ");
	for(int i=0; i < TIME_STAMP_BYTES_CNT;i++) {

		recv_buffer[i] = udp_buffer[index+i];
		printf("%d ", recv_buffer[i]);
	}

	struct tm *recv_dt = malloc(sizeof(struct tm));

	recv_dt->tm_mday = recv_buffer[0];
	recv_dt->tm_mon = recv_buffer[1];
	recv_dt->tm_year = recv_buffer[2];
	recv_dt->tm_hour = recv_buffer[3];
	recv_dt->tm_min = recv_buffer[4];
	recv_dt->tm_sec = recv_buffer[5];

	time_t t1 = mktime(recv_dt);
	char date[500];
	if(t1 == -1) {
		printf("Unable to represent received time in UTC using mktime\n");
	}
	else {
		strftime(date, sizeof(date), "%c", recv_dt);
		printf("date: %s\n", date);
	}

	char *root_path = "/opt/Testing/Data";
	get_ModifiedFiles(root_path);

	prepare_udp_resp_body();


}