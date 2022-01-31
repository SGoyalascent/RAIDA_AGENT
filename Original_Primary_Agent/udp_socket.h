#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mysql.h>
#include "raida_server.h"
#include "aes.h"

//--------------------------------------------------------------------
#define FRAME_TIME_OUT_SECS		1 
#define UDP_BUFF_SIZE 			65535
//------------------------------------------------------------------
#define REQUEST_HEADER_MAX 		48
#define RESPONSE_HEADER_MAX 		65535
//-----------------------------------------------------------------
//Indexs at which the bytes start
#define REQ_CL  					0
#define REQ_SP  					1
#define REQ_RI  					2
#define REQ_SH  					3
#define REQ_CM  					4
#define REQ_VE  					6
#define REQ_CI  					7
#define REQ_RE  					9
#define REQ_EC  					12
#define REQ_FC  					14
#define REQ_EN  					16

#define REQ_NO_1  				9
#define REQ_NO_2  				10
#define REQ_NO_3  				11
#define REQ_NO_4  				12
#define REQ_NO_5  				13

#define REQ_NO_6				19
#define REQ_NO_7				20
#define REQ_NO_8				21

#define REQ_HEAD_MIN_LEN 		22
//-------------------------------------------------------------
#define TY_BYTES_CNT				1
#define TT_BYTES_CNT				1
#define RA_BYTES_CNT				1
#define DENOM_BYTES_CNT			1
#define PAGE_NO_BYTES_CNT		1
#define RAIDA_BYTE_CNT			1
#define ROWS_BYTES_CNT			1
#define YY_BYTES_CNT				1
#define MM_BYTES_CNT			1
#define DD_BYTES_CNT				1
#define COIN_TYPE_BYTES_CNT		1
#define MFS_BYTES_CNT			1
#define FREE_ID_BYTES_CNT			1
#define CMD_END_BYTES_CNT		2
#define SN_BYTES_CNT				3	
#define OWNER_ID_BYTES_CNT		3
#define RECORD_BYTES_CNT			3
#define KEY_ID_BYTES_CNT			3
#define TK_BYTES_CNT				4
#define MS_BYTES_CNT			4
#define AMT_BYTES_CNT			4
#define HS_BYTES_CNT				4
#define EN_BYTES_CNT				5
#define TIME_STAMP_BYTES_CNT		6
#define FREE_ID_SERV_LOCK_TIME		6
#define DT_BYTES_CNT				7	
#define RESP_BUFF_MIN_CNT			12
#define FIX_SRNO_MATCH_CNT		13
#define ENCRYPTION_CONFIG_BYTES  16
#define NOUNCE_BYTES_CNT         16
#define GUID_BYTES_CNT 			16
#define AN_BYTES_CNT 			16
#define PAN_BYTES_CNT 			16
#define PG_BYTES_CNT	 			16
#define KEY_BYTES_CNT			16
#define CH_BYTES_CNT				16
#define PWD_BYTES_CNT			16
#define LEGACY_RAIDA_TK_BYTES_CNT		22
#define MD_HASH_BYTES_CNT		32
#define META_DATA_BYTES_CNT		50
#define REQ_END					62
#define KEY_HASH_BYTES_CNT		64

//-------------------------------------------------
#define KEYS_MAX				100
#define ROWS_MUL_FACTOR			100
#define MAX_ROWS_SHOW_STAT		255	
#define FRAMES_MAX				100
#define EN_CODES_MAX			255
#define DEFAULT_YEAR			2000
#define COINS_MAX				2000
#define SECS_IN_DAY				60 * 60 *24
//#define SECS_IN_DAY				1

//------Indexs for Response Header----------------------------
#define RES_RI  						0
#define RES_SH  						1
#define RES_SS 	 					2
#define RES_EX 						3
#define RES_RE 						4
#define RES_EC 						6
#define RES_HS 						8
//---------Status Error codes----------------------------------------
#define INVALID_CLOUD_ID 		   				1
#define RAIDA_ OFFLINE 			   			2
#define INVALID_FRAME_CNT 	 				15
#define INVALID_PACKET_LEN 	 				16
#define FRAME_TIME_OUT	 	 				17
#define WRONG_RAIDA 			 				18
#define INVALID_SPLIT_ID 		 				19
#define SHARD_NOT_AVAILABLE   					20
#define VALIDATE_TICKET_FOUND					21
#define VALIDATE_TICKET_NOT_FOUND				22
#define VALIDATE_TICKET_INVALID_RAIDA			23
#define INVALID_CMD 			  				24
#define COIN_ID_NOT_FOUND 	  				25
#define COIN_LIMIT_EXCEED			 	  		26
#define INVALID_EN_CODE		 	  			27
#define COIN_OWNER_ID_NOT_FOUND		 	  	28
#define POWN_AN_PAN_SAME					30
#define LEN_OF_BODY_CANT_DIV_IN_COINS			32
#define INVALID_END_OF_REQ					33
#define EMPTY_REQ_BODY						36
#define VALIDATE_TICKET_CLAIMED_EARLIER			37
#define VALIDATE_TICKET_CLAIMED				38
#define COIN_NO_NOT_FOUND					39
#define SN_ALL_READY_IN_USE					40
#define SERVICE_LOCKED						41
#define FAILED_TO_AUTHENTICATE					64
#define PAGE_NOT_FOUND						66
#define BREAK_COUNTER_FEIT					70
#define BREAK_COINS_NOT_FOUND					72
#define BREAK_COINS_SUM_NOT_MATCH				74
#define BREAK_CANNOT_BREAK					76
#define JOIN_COUNTER_FEIT						80
#define JOIN_COINS_NOT_FOUND					82
#define JOIN_COINS_SUM_NOT_MATCH				84
#define JOIN_CANNOT_JOIN						86
#define FIX_ALL_TICKET_ZERO					90
#define LEGACY_RAIDA_TIME_OUT					100
#define LEGACY_RAIDA_FAIL						101
#define IDENTIFY_COIN_FOUND					192
#define IDENTIFY_COIN_NOT_FOUND				193
#define SYNC_ADD_COIN_EXIST					200
#define FIND_ALL_NONE						208
#define FIND_ALL_AN							209
#define FIND_ALL_PA							210
#define FIND_MIXED							211
#define ALL_PASS							241
#define ALL_FAIL								242
#define MIX								243
#define SUCCESS								250
#define FAIL								251
#define NO_ERR_CODE	 		   				255
//----------Coin Converter Command Codes---------------------
#define NO_RESPONSE                    244
#define NO_TICKET_FOUND                245
//----------Command codes-----------------------------------------
#define CMD_COIN_CONVERTER 					215
#define CMD_ECHO							4

#define STATE_WAIT_START						1
#define STATE_START_RECVD					2
#define STATE_WAIT_END						3
#define STATE_END_RECVD						4
//--------------------------------------------------------------------------	
#define  UDP_RESPONSE 						0
#define  FIFO_RESPONSE 						1

extern int sockfd;
extern unsigned char response_flg;
extern int32_t key_cnt;
extern fd_set select_fds;                
extern struct timeval timeout;
extern struct sockaddr_in servaddr, cliaddr;
extern long time_stamp_before,time_stamp_after;
extern unsigned char udp_buffer[UDP_BUFF_SIZE], response[RESPONSE_HEADER_MAX],EN_CODES[EN_CODES_MAX];

union coversion{
	uint32_t val32;
	unsigned char data[4];
};
extern union coversion snObj;

union serial_no {
	uint32_t val;
	unsigned char buffer[3];
};
extern union serial_no sn_no;

//------------------------------------------------------------------------
int listen_request(); 
void* listen_request_raida(void *arg);
int init_udp_socket();
int load_encrypt_key();
void set_time_out(unsigned char);
void process_request(unsigned int);
void execute_coin_converter(unsigned int);
void send_response(unsigned char ,unsigned int );
void send_err_resp_header(int );
void execute_echo(unsigned int);
void print_udp_buffer(int);
void decrypt_request_body(int);
unsigned char validate_request_header(unsigned char *,int );
unsigned char validate_request_body_general(unsigned int,unsigned int ,int *);
unsigned char validate_request_body(unsigned int ,unsigned char,unsigned int ,int *);
#endif
