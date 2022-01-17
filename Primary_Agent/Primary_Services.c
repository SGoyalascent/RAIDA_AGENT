#include "RAIDA_Agent.h"

int sockfd;
fd_set select_fds;               
struct timeval timeout;
struct sockaddr_in servaddr, cliaddr;
long time_stamp_before,time_stamp_after;
unsigned char free_thread_running_flg;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_SIZE_MAX], udp_response[MAXLINE];
unsigned int index_resp = RES_HS + HS_BYTES_CNT;
int total_frames = 0
char execpath[256], file_path[500];
time_t t1;


//-------------------------------------------------
//Get the Working Directory
//------------------------------------------------
void get_execpath() {
    strcpy(execpath, "/opt/Testing/Data");
    printf("Working_Dir_path: %s\n", execpath);
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
	// Creating socket file descriptor
	printf("init_udp_socket\n");
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	// Filling server information
	servaddr.sin_family = AF_INET; // IPv4
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(Primary_agent_config.port_number);
	// Bind the socket with the server address
	if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 ){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
}
//-----------------------------------------------------------
// receives the UDP packet from the client
//-----------------------------------------------------------
int listen_request(){
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
				n = recvfrom(sockfd, (unsigned char *)buffer, server_config_obj.bytes_per_frame,MSG_WAITALL,(struct sockaddr *) &cliaddr,&len);
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
						printf("Invalid end of packet  \n");
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
//-----------------------------------------------------------
// Processes the UDP packet 
//-----------------------------------------------------------
void process_request(unsigned int packet_len){
	uint16_t cmd_no=0, coin_id;
	time_stamp_before = get_time_cs();
	memset(response,0,RESPONSE_SIZE_MAX-1);
	cmd_no = udp_buffer[REQ_CM+1];
	cmd_no |= (((uint16_t)udp_buffer[REQ_CM])<<8);
	switch(cmd_no){
	
		case MIRROR_REPORT_CHANGES : 		execute_Report_Changes(packet_len);break;
		case AGENT_GET_PAGE : 				execute_Mirror_Get_Page(packet_len);break;
		case CMD_ECHO:						execute_echo(packet_len);break;
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
	response[RES_RE+1] = 1;    // response frames count 
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
	len=sizeof(cliaddr);
	
	sendto(sockfd, (const char *)response,size,MSG_CONFIRM,(const struct sockaddr *) &cliaddr,len);	
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
//  Validate request body 
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body_general(unsigned int packet_len,unsigned int req_body,int *req_header_min){
	*req_header_min = REQ_HEAD_MIN_LEN;
	if(packet_len != (*req_header_min) + req_body){
		send_err_resp_header(INVALID_PACKET_LEN);
		return 0;
	}
	return 1;
}
//------------------------------------------------------------------------
//SEND RESPONSE
//------------------------------------------------------------------------
void Send_Response(unsigned int size){
	int len = sizeof(cliaddr);
	char * myfifo = "/tmp/myfifo";
	//prepare_resp_header(status_code);
	sendto(sockfd, (const char *)udp_response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
}
//------------------------------------------------------------------
//PREPARE RESPONSE HEADER
//----------------------------------------------------------------
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
	response[RES_RE+1] = total_frames;    //frame count
	response[RES_EC] = udp_buffer[REQ_EC];
	response[RES_EC+1] = udp_buffer[REQ_EC+1];
	response[RES_HS] = 0;
	response[RES_HS+1] = 0;
	response[RES_HS+2] = 0;
	response[RES_HS+3] = 0;
 
}   
//---------------------------------------------------------------
//PREPARE UDP RESPONSE BODY FOR REPORT CHANGES SERVICE
//--------------------------------------------------------------
void prepare_udp_resp_body() {

    unsigned char status_code;
    unsigned int size = 0;
    
    if(index_resp == RESP_BUFF_MIN_CNT) {
		status_code = RAIDA_AGENT_NO_CHANGES;  //MIRROR_REQUESTED_FILE_NOT_EXIST
        total_frames = 0;
        prepare_resp_header(status_code);
        memcpy(udp_response, response, index_resp);
		size = RES_HS + HS_BYTES_CNT;
		Send_Response_toPrimaryAgent(size);
		return;
	}
    
    response[index_resp+0] = 0x3E;
    response[index_resp+1] = 0x3E;

    int resp_length = index_resp + RESP_BODY_END_BYTES;
    unsigned int current_length = resp_length;
    printf("current_length: %u ", current_length);

    total_frames = (resp_length/MAXLINE) + 1;
    if((resp_length % MAXLINE) == 0) {
        total_frames = total_frames - 1;
    }
    printf("resp_length: %d  current_length: %u total_frames: %d\n", resp_length, current_length, total_frames);

    int frames = 0, index = 0;
    status_code = MIRROR_REPORT_RETURNED;
    prepare_resp_header(status_code);
    while(frames < total_frames) {
        
        frames++;
        
        if(current_length <= MAXLINE) {
            memcpy(udp_response, &response[index], current_length);
            index += current_length;
            size = current_length;
            Send_Response_toPrimaryAgent(size);
        }
        else {
            memcpy(udp_response, &response[index], MAXLINE);
            index += MAXLINE;
            size = MAXLINE;
            current_length = current_length - MAXLINE;
            Send_Response_toPrimaryAgent(size);
        }
        printf("current_length: %u frames: %d ", current_length, frames);
    }
}
//---------------------------------------------------------------
//PREPARRE RESPONSE FOR REPORT CHANGES SERVICE
//--------------------------------------------------------------
int prepare_resp_body(int index) {
    
    byteObj.val32 = coin_id;
    response[index+0] = byteObj.byte2[1]; //MSB
    response[index+1] = byteObj.byte2[0];  //LSB
    
    response[index+2] = table_id;

    byteObj.val32 = serial_no;
    response[index+3] = byteObj.byte4[3]; // msb
    response[index+4] = byteObj.byte4[2];
    response[index+5] = byteObj.byte4[1];
    response[index+6] = byteObj.byte4[0]; // lsb
    
    return (index+7);
}
//-----------------------------------------------------------------------
//Get File Nmaes which need to be Synchronised
//----------------------------------------------------------------------
void get_ModifiedFiles(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;

	int root_path_len = strlen(execpath);
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char df_path[500], df_name[256];
        sprintf(df_name, "%s",dir->d_name);
        sprintf(df_path, "%s/%s", path, dir->d_name);

        if(dir->d_type == DT_REG) {
            
			printf("filename: %s  filepath: %s\n", df_name, df_path);
            time_t t2;
            char datestring[256], sub_path[500];
            double time_dif;

            if(stat(df_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            strcpy(sub_path, &df_path[root_path_len+1]);
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
            
            if(strcmp(coin, "Owners") == 0) {  
                coin_id = 254;
                table_id = 0;
                serial_no = atoi(df_name); 
                continue;
            }

            if(strcmp(coin, "my_id_coins") == 0) {
                coin_id = 255;
                table_id = 0;
                serial_no = atoi(df_name); 
                continue;
            }

            token = strtok(NULL, "/");
            strcpy(table, token);

            token = strtok(coin, "_");
            token = strtok(NULL,"_");
            strcpy(c_id, token);
            coin_id = atoi(c_id);
            serial_no = atoi(df_name);   //12345.bin file -->  12345

            if(strcmp(table, "ANs") == 0) {
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

			index_resp = prepare_resp_body(index_resp);
        }	
			
        if((dir->d_type == DT_DIR) && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("dir_name: %s  dir_path: %s\n", df_name,df_path);
            get_ModifiedFiles(df_path); 
        }
    }
    closedir(d);
}
//-------------------------------------------------------------------
//EXECUTE RREPORT CHANGES SERVICE
//------------------------------------------------------------------
void  execute_Report_Changes(unsigned int packet_len) {

	int req_body_bytes = CH_BYTES_CNT + CMD_END_BYTES_CNT + TIMESTAMP_BYTES_CNT, req_header_min, index=0;
	unsigned char recv_buffer[TIMESTAMP_BYTES_CNT];

	printf("------------REPORT CHANGES SERVICE-------------------\n");
	if(validate_request_body_general(packet_len,req_body_bytes,&req_header_min)==0){
		send_err_resp_header(EMPTY_REQ_BODY);
		return;
	}

	index = req_header_min + CH_BYTES_CNT;
	printf("recv_buffer: ");
	for(int i=0; i < TIMESTAMP_BYTES_CNT;i++) {
		recv_buffer[i] = udp_buffer[index+i];
		printf("%d ", recv_buffer[i]);
	}

	struct tm *recv_dt = malloc(sizeof(struct tm));

	recv_dt->tm_year = recv_buffer[0];
    recv_dt->tm_mon = recv_buffer[1];
    recv_dt->tm_mday = recv_buffer[2];
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
		printf("Last Modified Time(UTC):- %d-%d-%d  %d:%d:%d\n", recv_dt->tm_mday,recv_dt->tm_mon+1,recv_dt->tm_year+1900, 
                                                                                recv_dt->tm_hour,recv_dt->tm_min, recv_dt->tm_sec );
	}

	char *root_path;
	strcpy(root_path, execpath);

	get_ModifiedFiles(root_path);

	prepare_udp_resp_body();
}

void execute_Mirror_Get_Page(unsigned int packet_len) {

    int req_body_bytes = CH_BYTES_CNT + CMD_END_BYTES_CNT + RAIDA_AGENT_FILE_ID_BYTES_CNT;
	int req_header_min;
	unsigned int index=0;
	unsigned char recv_buffer[RAIDA_AGENT_FILE_ID_BYTES_CNT];
    unsigned int coin_id, table_id;

	if(validate_request_body_general(packet_len,req_body_bytes,&req_header_min)==0){
		send_err_resp_header(EMPTY_REQ_BODY);
		return;
	}

	index = req_header_min + CH_BYTES_CNT;
	printf("recv_buffer: ");

	//bytes sent in msb to lsb order, so [0] = msb, [1] = lsb;  [3] = msb, [6] = lsb
	for(int i=0; i < RAIDA_AGENT_FILE_ID_BYTES_CNT;i++) {

		recv_buffer[i] = udp_buffer[index+i];
		printf("%d ", recv_buffer[i]);
	}

    bytes.byte_coin[0] = recv_buffer[1]; //lsb
    bytes.byte_coin[1] = recv_buffer[0];  //msb

    coin_id = bytes.val;
    table_id = recv_buffer[2];

    bytes.byte_sn[0] = recv_buffer[6];
    bytes.byte_sn[1] = recv_buffer[5];
    bytes.byte_sn[2] = recv_buffer[4];
    bytes.byte_sn[3] = recv_buffer[3];

    serial_no = bytes.val;

    char root_path[256] = "/opt/Testing/Data";
    char filepath[500];

    strcpy(filepath, root_path);
    //coin_id
    strcat(filepath, "/coin_");
    char c_id[20];
    sprintf(c_id, "%d", coin_id);
    strcat(filepath, c_id);
    //table_id
    if(table_id == 0) {
        strcat(filepath, "/ANs");  
    }
    else if(table_id == 1) {
        strcat(filepath, "/Owners");
    }
    else if(table_id == 2) {
        strcat(filepath, "/Statements");
    }
    else if(table_id == 3) {
        strcat(filepath, "/Loss_Coin_Report");
    }
    else if(table_id == 4) {
        strcat(filepath, "/Email_Recover");
    }
    else {
        printf("table_id: %d\n", table_id);
        printf("Error: Wrong table_id");
    }

    //serial_no

    char sn_no[20];
    sprinf(sn_no, "%d", serial_no);
    strcat(filepath, "/");
    strcat(filepath, sn_no);
    strcat(filepath, ".bin");

    printf("filepath: %s\n", filepath);

	strcpy(file_path, filepath);

	Get_File_Contents();

}


void Get_File_Contents() {

    FILE *fp_inp = NULL;
    int ch, size = 0;
	//char file_path[500];

    fp_inp = fopen(file_path, "rb");
    if(fb_inp == NULL) {
        printf("%d.bin cannot be opened, exiting\n", serial_no);
        return;
    }

    while((ch = fgetc(fp_inp) ) != EOF) {
        size++;
    }
	printf("file_size: %d\n", size);
    fclose(fp_inp);

    fp_inp = fopen(file_path, "rb");
    if(fread(&response[RESP_BUFF_MIN_CNT], 1, size, fp_inp) < size) {
        printf("Contents missing in the %d.bin file\n", serial_no);
        return;
    }
    fclose(fp_inp);

    printf("contents: ");
    for(int i=0; i <=size; i++) {
        printf("%d ", response[i]);
    }
    printf("\n");

    index_resp += size;

}