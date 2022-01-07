//Get Request from the Primary Raida Agent, 
//Process the Request
//Send the response

#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0, total_frames = 0, root_path_len = 0;
time_t t1;
unsigned char udp_buffer[UDP_BUFF_SIZE], response[RESPONSE_HEADER_MAX], udp_response[MAXLINE];
unsigned int coin_id, table_id, serial_no;
unsigned int index_resp = RES_HS + HS_BYTES_CNT;
char execpath[256];


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

void Send_Response_toPrimaryAgent(unsigned int size){
	int len = sizeof(cliaddr);
	char * myfifo = "/tmp/myfifo";
	//prepare_resp_header(status_code);
	sendto(sockfd, (const char *)udp_response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
}

void prepare_udp_resp_body() {

    unsigned char status_code;
    unsigned int size = 0;
    
    if(index_resp == RESP_BUFF_MIN_CNT) {
		status_code = RAIDA_AGENT_NO_CHANGES;
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

void get_Files_path() {



}

void get_ModifiedFiles(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    
	//char root_path[256] = "/opt/Testing/Data";
    //root_path_len = strlen(root_path);

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
            char datestring[256];
            double time_dif;
            char sub_path[500];

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
            //printf("datestring: %s\n", datestring);
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
	}

	char *root_path = "/opt/Testing/Data";
    root_path_len = strlen(root_path);

	get_ModifiedFiles(root_path);

	prepare_udp_resp_body();


}