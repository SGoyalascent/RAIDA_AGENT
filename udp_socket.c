#include "udp_socket.h"
#include "aes.h"

int sockfd;
fd_set select_fds;               
struct timeval timeout;
union coversion snObj; 
union serial_no sn_no;
struct sockaddr_in servaddr, cliaddr;
long time_stamp_before,time_stamp_after;
unsigned char udp_buffer[UDP_BUFF_SIZE],response[RESPONSE_HEADER_MAX],coin_table_id[5],EN_CODES[EN_CODES_MAX]={0};
unsigned char free_thread_running_flg;
uint8_t encrypt_key[ENCRYPTION_CONFIG_BYTES];
uint8_t nounce[NOUNCE_BYTES_CNT];
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
	servaddr.sin_port = htons(server_config_obj.port_number);
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

//----------------------------------------------------------
//Loads encrypt key from encryption_key.bin
//--------------------------------------------------------- 
int load_encrypt_key(){
	FILE *fp_inp = NULL;
	int i = 0;
	unsigned char buff[ENCRYPTION_CONFIG_BYTES];
	char path[256];
	strcpy(path,execpath);
	strcat(path,"/Data/encryption_key.bin");
	printf("------------------------------\n");
	printf("ENCRYPTION CONFIG KEY\n");
	//printf("------------------------------\n");
	if ((fp_inp = fopen(path, "rb")) == NULL) {
		printf("encryption_key.bin.bin Cannot be opened , exiting \n");
		return 1;
	}
	if(fread(buff, 1, ENCRYPTION_CONFIG_BYTES, fp_inp)<(ENCRYPTION_CONFIG_BYTES)){
		printf("Configuration parameters missing in encryption_key.bin \n");
		return 1;
	}
	memcpy(encrypt_key,buff,ENCRYPTION_CONFIG_BYTES);
	for(i=0;i<ENCRYPTION_CONFIG_BYTES;i++){
	 	printf("%02x ",encrypt_key[i]);
	}
	printf("\n");
	fclose(fp_inp);
	
	memset(nounce,0,NOUNCE_BYTES_CNT);
	//We take nouce 5 bytes
	for(int i=0;i < 5;i++){
		nounce[i] = udp_buffer[REQ_NO_1+i];
	}
	int j=0;
	//We take nouce 3 bytes 
	for(int i=5; i < 8;i++){
		nounce[i] = udp_buffer[REQ_NO_6+j];
		j++;
	}
	for(int i =0; i < NOUNCE_BYTES_CNT; i++) {
		printf("%d  ", nounce[i]);
	}
	return 0;
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
//------------------------------------------------------------------------------------------
//  Validate coins and request body and return number of coins 
//-----------------------------------------------------------------------------------------
unsigned char validate_request_body(unsigned int packet_len,unsigned char bytes_per_coin,unsigned int req_body_without_coins,int *req_header_min){
	unsigned int no_of_coins=0;
	*req_header_min = REQ_HEAD_MIN_LEN;
	no_of_coins = (packet_len-(*req_header_min+req_body_without_coins))/bytes_per_coin;
	printf("---------------Validate Request Body---------------------------\n");
	if((packet_len-(*req_header_min+req_body_without_coins))%bytes_per_coin!=0){
		send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins==0){
		send_err_resp_header(LEN_OF_BODY_CANT_DIV_IN_COINS);
		return 0;
	}
	if(no_of_coins>COINS_MAX){
		send_err_resp_header(COIN_LIMIT_EXCEED);
		return 0;
	}
	printf("Number of coins = :  %d \n", no_of_coins);	
	return no_of_coins;
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
//---------------------------------------------------------------
//	SEND RESPONSE
//---------------------------------------------------------------
void send_response(unsigned char status_code,unsigned int size){
	int len=sizeof(cliaddr);
	char * myfifo = "/tmp/myfifo";
	prepare_resp_header(status_code);
	sendto(sockfd, (const char *)response, size,
		MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
		len);
}

//---------------------------------------------------------------
// ECHO COMMAND  4
//---------------------------------------------------------------
void execute_echo(unsigned int packet_len){
	int req_body = 0,req_header_min=0,size;
	printf("ECHO Command \n");
	size    =  RES_HS+HS_BYTES_CNT;
	send_response(SUCCESS,size);
}
//-------------------------------------------------------------
//DECRYPT REQUEST BODY
//-------------------------------------------------------------
void decrypt_request_body(int n) {
	
	int req_body = n - 22;
	unsigned char *req_ptr = &udp_buffer[22];
	unsigned char *key = &encrypt_key[0];
	unsigned char *iv = &nounce[0];

	load_encrypt_key();
	crypt_ctr(key,req_ptr,req_body,iv);
}

void print_udp_buffer(int n) {
	printf("udp_buffer: ");
	for(int i = 0; i < n; i++) {
		printf("%d  ", udp_buffer[i]);
	}
}

//---------------------------------------------------------------
//Coin converter 215
//---------------------------------------------------------------

void execute_coin_converter(unsigned int packet_len) {
	int req_body = CH_BYTES_CNT + CMD_END_BYTES_CNT + LEGACY_RAIDA_TK_BYTES_CNT;
	int req_header_min;
	unsigned int index=0,size=0;
	unsigned char status_code, ticket_buffer[22];

	printf("--------------COIN CONVERTER COMMAND-------------- \n");

	if(validate_request_body_general(packet_len,req_body,&req_header_min)==0){
		send_err_resp_header(EMPTY_REQ_BODY);
		return;
	}

	index = req_header_min + CH_BYTES_CNT;
	printf("req_header_min: %d  index: %u\n", req_header_min, index);
	printf("buffer:  ");
	for(int j=0;j<LEGACY_RAIDA_TK_BYTES_CNT;j++) {
		ticket_buffer[j] = udp_buffer[index+(LEGACY_RAIDA_TK_BYTES_CNT-1-j)]; 
		printf("%d  ", ticket_buffer[j]);
	} 
	printf("\n");
	unsigned char ticket_hex_bytes[45];
	char* ptr =  &ticket_hex_bytes[0];
	for(int j=0;j<LEGACY_RAIDA_TK_BYTES_CNT;j++) {
		ptr += sprintf(ptr, "%02x", udp_buffer[index+(LEGACY_RAIDA_TK_BYTES_CNT-1-j)]);
	}
	printf("Ticket_no.:- %s\n", ticket_hex_bytes); 
	index = RES_HS+HS_BYTES_CNT;
	size = RES_HS+HS_BYTES_CNT;

	//------- READ COIN_CONVERTER CONFIG FILE---------------------

	char Host_ip[256], Database_name[256], Username[256], User_password[256], Encryption_key[256], Mode[256];
	int listen_port;

    printf("Welcome to MySql Database\n");
    FILE *myfile = fopen("Coin_Converter.config", "r");
    if(myfile == NULL) {
        printf("Config file not found\n");
		return;
    }
    fscanf(myfile, "Host = %255s Database = %255s Username = %255s Password = %255s listenport = %d encryption_key = %255s mode = %255s", Host_ip, Database_name,
                                                  Username, User_password, &listen_port, Encryption_key, Mode);
    fclose(myfile);
    //printf("Host = %s\t\t Database = %s\t\t Username = %s\t\t Password = %s\t\t listenport = %d\t\t encryption_key = %s\t\t mode = %s\n", Host_ip, Database_name, Username, User_password, listen_port, Encryption_key, Mode);

// -----------------------Initialize a connection to the Database---------------------

	MYSQL *con = mysql_init(NULL);
    if(con == NULL) {
        printf("Error_code: %u\t stderr: %s\n",mysql_errno(con), mysql_error(con));
		status_code = FAIL;
		printf("Status_code: NO_RESPONSE\n");
		send_response(status_code,size);
		return;
	}

	if(mysql_real_connect(con, Host_ip, Username, User_password, Database_name, listen_port, NULL, 0) == NULL) {
		printf("Error_code: %u\t stderr: %s\n",mysql_errno(con), mysql_error(con));
		mysql_close(con);
		status_code = FAIL;
		printf("Status_code: NO_RESPONSE\n");
		send_response(status_code,size);
		return;
    }

//SELECT statement
	unsigned char query1[256];
	unsigned char query2[256];
	unsigned char query3[256];
	MYSQL_RES *result;
	unsigned int sr_nos_size;

	sprintf(query1, "SELECT sn FROM fixit_log WHERE rn = '%s'", ticket_hex_bytes);
	mysql_query(con, query1);
	if(mysql_errno(con) != 0) {
        printf("stderr: %s\n", mysql_error(con));
		mysql_close(con);
		status_code = FAIL;
		printf("Status_code: NO_RESPONSE\n");
		send_response(status_code,size);
		return;
    }

	result = mysql_store_result(con);
	if( mysql_errno(con) != 0) {
		printf("stderr: %s\n", mysql_error(con));
		mysql_close(con);
		status_code = FAIL;
		printf("Status_code: NO_RESPONSE\n");
		send_response(status_code,size);
		return;
	}

	sr_nos_size = mysql_num_rows(result);
	printf("No. of Rows: %d\n", sr_nos_size);
	if(sr_nos_size == 0) {
		status_code = NO_TICKET_FOUND;
		printf("Status_code: NO_TICKET_FOUND\n");
		send_response(status_code,size);
		return;

	}
	if(sr_nos_size > 300) {
		sr_nos_size = 300;
	}

	int k = 0;
	for(int i =0; i < sr_nos_size; i++) {
		MYSQL_ROW row = mysql_fetch_row(result);                                    
		sscanf(row[0], "%u", &sn_no.val);
		printf("k: %d\t --sn_no.val: %u\n",k, sn_no.val);
		k++;

		for(int j = 0; j < SN_BYTES_CNT; j++) {
			response[index+(3*i)+j] = sn_no.buffer[SN_BYTES_CNT-1-j];
			printf(" res[%d]: %d", index+(3*i)+j, response[index+(3*i)+j]);
		} 
		printf("\n");
	/*
		sprintf(query2, "DELETE FROM fixit_log WHERE sn = '%u'", sn_no.val);
		if(mysql_query(con, query2)) {
			printf("Failed to Delete record successfully\n")
			printf("stderr: %s\n", mysql_error(con));
			mysql_close(con);
			return;
		}

		sprintf(query3, "UPDATE ans SET NN = 2 WHERE SN = '%u' AND NN = 1", sn_no.val);
		if(mysql_query(con, query3)) {
			printf("stderr: %s\n", mysql_error(con));
			mysql_close(con);
			return;
		} */
	}

	mysql_free_result(result);
	mysql_close(con);

	status_code = SUCCESS;
	index = RES_HS+HS_BYTES_CNT;
	unsigned int response_body_size = SN_BYTES_CNT*sr_nos_size;
	size = RES_HS + HS_BYTES_CNT + response_body_size;
	unsigned char *key = &encrypt_key[0];
	unsigned char *iv = &nounce[0];
	unsigned char *resp_ptr = &response[index];
	
	crypt_ctr(key,resp_ptr,response_body_size,iv);
	send_response(status_code,size);
	

}
