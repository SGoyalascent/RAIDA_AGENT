#include "RAIDA_Agent.h"

unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_SIZE_MAX], udp_response[MAXLINE];
unsigned int index_resp = RES_HS + HS_BYTES_CNT;
int total_frames = 0
char execpath[256];
char file_path[500];


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

void Send_Response_PrimaryAgent(unsigned char status_code,unsigned int size){
	int len=sizeof(cliaddr);
	char * myfifo = "/tmp/myfifo";
	prepare_resp_header(status_code);
	sendto(sockfd, (const char *)udp_response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,len);
}

void prepare_resp_header(unsigned char status_code){
	unsigned char ex_time;
	time_stamp_after = get_time_cs();
	if((time_stamp_after-time_stamp_before) > 255){
		ex_time = 255;
	}else{
		ex_time= time_stamp_after-time_stamp_before;
	}

	udp_response[RES_RI] = server_config_obj.raida_id;
	udp_response[RES_SH] = 0;
	udp_response[RES_SS] = status_code;
	udp_response[RES_EX] = ex_time;
	udp_response[RES_RE] = 0;
	udp_response[RES_RE+1] = total_frames;
	udp_response[RES_EC] = udp_buffer[REQ_EC];
	udp_response[RES_EC+1] = udp_buffer[REQ_EC+1];
	udp_response[RES_HS] = 0;
	udp_response[RES_HS+1] = 0;
	udp_response[RES_HS+2] = 0;
	udp_response[RES_HS+3] = 0;

}   

void prepare_udp_resp_body() {

    unsigned char status_code;
    unsigned int size = 0;
	
	if(index_resp == RESP_BUFF_MIN_CNT) {
		status_code = MIRROR_REQUESTED_FILE_NOT_EXIST;
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
    status_code = SUCCESS;
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