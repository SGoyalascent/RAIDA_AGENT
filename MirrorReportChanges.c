//Get Request from the Primary Raida Agent, 
//Process the Request
//Send the response

#include "RAIDA_Agent.h"

struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX];

#define SN_SIZE  14


//-----------------------------------------------------------
//Initialize UDP Socket and bind to the port
//-----------------------------------------------------------
int init_udp_socket(){}

void ListenRequest_PrimaryAgent() {}

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
	prepare_resp_header(status_code);
	sendto(sockfd, (const char *)response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		len);
}

void show_dir_content(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    char datestring[256];
    struct tm *dt;
    time_t t2;
    double time_dif;
	char org_path[256] = "/opt/Testing/Data";
    unsigned char coin_id[10];
    unsigned char serial_no[10];
    int path_len = strlen(org_path);
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        // if the type is not directory
        if(dir->d_type == DT_REG) {
            
            char f_path[500];
            char filename[256];
			char sub_path[500];
            char coin[20];
            char table[20];
            char serial[20];
            sprintf(filename, "%s",dir->d_name);
            sprintf(f_path, "%s/%s", path, dir->d_name);
            printf("filename: %s", filename);
            printf("  filepath: %s\n", f_path);

            if(stat(f_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s\n", datestring);

            time_dif = difftime(t2, t3);
            printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
				printf("File Updated.  ");
                printf("datestring: %s  ", datestring);

                //printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month,tm.year, tm.hour, tm.minutes, tm.second);
                printf("Last Modified Time(UTC):- %d-%d-%d  %d:%d:%d\n",dt->tm_mday,dt->tm_mon,dt->tm_year+1900, dt->tm_hour,dt->tm_min, dt->tm_sec);

				strcpy(sub_path, f_path+path_len+1);
            	printf("sub_path: %s\n", sub_path);

				if(strcmp(sub_path, filename) != 0) {
                char *token;
                token = strtok(sub_path, "/");
                strcpy(coin, token);

                token = strtok(NULL, "/");
                strcpy(table, token);

                token = strtok(NULL, "/");
                strcpy(serial, token);

                
                token = strtok(coin, "_");
                token = strtok(NULL,"_");
                strcpy(coin_id, token);
                printf("coin_id: %s  ", coin_id);
                int c = atoi(coin_id);
                printf("c_id: %d\n", c_id);

                token = strtok(serial, ".");
                strcpy(serial_no, token);
                printf("serial_no: %s  ", serial_no);
                int sn_no = atoi(serial);
                printf("sn_no: %d\n", sn_no);
            }
			
			
			
			
			
			}
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("directory: %s ", dir->d_name);
            char d_path[500]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
            printf("  dirpath: %s\n", d_path);
            show_dir_content(d_path); // recall with the new path
        }
    }
    closedir(d);
}


void  CheckChanges() {

	int req_body_bytes = CH_BYTES_CNT + CMD_END_BYTES_CNT + TIMESTAMP_BYTES_CNT;
	int req_header_min;
	unsigned int index=0,size=0;
	unsigned char status_code;
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
	show_dir_content(root_path);


}