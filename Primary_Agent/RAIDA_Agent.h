#ifndef RAIDA_AGENT_H
#define RAIDA_AGENT_H
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>

//#include "Agent_Services.h"

//--------------------------------------------------------------------
#define FRAME_TIME_OUT_SECS		    3 
#define RESPONSE_TIME_OUT_SECS      10
#define UDP_BUFF_SIZE 			    65535
#define REQUEST_HEADER_MAX 		    48
#define MAXLINE                     1024
#define RESPONSE_SIZE_MAX 		    65535

#define VER                         255
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
#define REQ_ID                      17
#define REQ_SN                      19

#define REQ_NO_1  				9
#define REQ_NO_2  				10
#define REQ_NO_3  				11
#define REQ_NO_4  				12
#define REQ_NO_5  				13

#define REQ_NO_6				19
#define REQ_NO_7				20
#define REQ_NO_8				21

#define REQ_HEAD_MIN_LEN 		22
#define REQ_END					62
//-------------------------------------------------------------

#define PAGE_NO_BYTES_CNT		1
#define RAIDA_BYTE_CNT			1
#define ROWS_BYTES_CNT			1

#define YY_BYTES_CNT				1
#define MM_BYTES_CNT			    1
#define DD_BYTES_CNT				1

#define CMD_END_BYTES_CNT		    2
#define RESP_BODY_END_BYTES         2
#define SN_BYTES_CNT				3	
#define KEY_ID_BYTES_CNT			3
#define HS_BYTES_CNT				4
#define EN_BYTES_CNT				5
#define RESP_BUFF_MIN_CNT			12

#define ENCRYPTION_CONFIG_BYTES     16
#define NOUNCE_BYTES_CNT            16
#define KEY_BYTES_CNT			    16
#define CH_BYTES_CNT				16
//-------------------------------------------------
#define KEYS_MAX				100
#define FRAMES_MAX				100
#define EN_CODES_MAX			255
#define DEFAULT_YEAR			2000
#define SECS_IN_DAY				60 * 60 *24				1

//------Indexs for Response Header----------------------------
#define RES_RI  					0
#define RES_SH  					1
#define RES_SS 	 					2
#define RES_EX 						3
#define RES_RE 						4
#define RES_EC 						6
#define RES_HS 						8

#define RESP_HEADER_MIN_LEN         12
#define RESP_END                    62
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

#define RAIDA_AGENT_PAGES_RETURNED          100
#define RAIDA_AGENT_NO_CHANGES              101
#define RAIDA_AGENT_PRIMARY_UP              102
#define RAIDA_AGENT_PRIMARY_DOWN            104
#define MIRROR_MESSAGE_RECEIVED             105
#define MIRROR_REPORT_RETURNED              106
#define MIRROR_REQUESTED_FILE_NOT_EXIST     107

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
//----------Command codes----------------------------------------
#define CMD_ECHO							4
#define AGENT_GET_PAGE                      40
#define MIRROR_REPORT_CHANGES               45
//-------------------------------------------------------------
#define STATE_WAIT_START					1
#define STATE_START_RECVD					2
#define STATE_WAIT_END						3
#define STATE_END_RECVD						4
//--------------------------------------------------------------------------	
#define  UDP_RESPONSE 						0
#define  FIFO_RESPONSE 						1

//-----------RAIDA Agent Codes---------------------------------------

#define TABLE_ID_NO_TABLE               0
#define TABLE_ID_ANS                    1
#define TABLE_ID_OWNERS                 2
#define TABLE_ID_STATEMENTS             3 
#define TABLE_ID_LOSS_COIN_REPORT       4
#define TABLE_ID_EMAIL_RECOVER          5

#define COIN_ID_COIN_OWNERS             254
#define COIN_ID_MY_ID_COINS             255

#define RAIDA_AGENT_TABLE_ID_BYTES_CNT          1
#define RAIDA_AGENT_COIN_ID_BYTES_CNT           2
#define RAIDA_AGENT_SN_NO_BYTES_CNT             4
#define TIMESTAMP_BYTES_CNT		                6
#define RAIDA_AGENT_FILE_ID_BYTES_CNT           7

#define AGENT_PRIMARY               1
#define AGENT_MIRROR                2
#define AGENT_WITNESS               3

#define SN_SIZE                     14
#define FILES_COUNT_MAX           10000  //9360
#define KEYS_COUNT                10000
#define AGENT_FRAMES_MAX           65

//------MAIN--------------------------
extern char execpath[256], serverpath[256], keys_bytes[KEYS_COUNT][KEY_BYTES_CNT];
extern time_t t1;
//-------CALL SERVICES-------------------

extern struct sockaddr_in servaddr;
extern int sockfd;
extern fd_set select_fds;  
extern struct timeval timeout;
extern unsigned char send_req_buffer[MAXLINE], recv_response[RESPONSE_SIZE_MAX];
extern unsigned char files_id[FILES_COUNT_MAX][RAIDA_AGENT_FILE_ID_BYTES_CNT], req_file_id[RAIDA_AGENT_FILE_ID_BYTES_CNT];
extern unsigned int total_files_count;

//--------------------------------------------
struct agent_config {
    char Ip_address[50];
    unsigned int port_number;
};
extern struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;

struct server_config {
	unsigned char raida_id;
	unsigned int bytes_per_frame;
};
extern struct server_config server_config_obj;

union conversion {
	uint32_t val32;
    unsigned int val;
    unsigned char byte2[2];
	unsigned char byte4[4];
};
extern union conversion byteObj;

struct timestamp {
    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minutes;
    unsigned char second;
};
extern struct timestamp tm;

//---------------MAIN--------------------------------------
void get_execpath();
void WelcomeMsg();
void Read_Agent_Configuration_Files();
void getcurrentpath();
int load_raida_no();
void get_latest_timestamp(char *);
void read_keys_file();

//---------------CALL SERVICES-----------------------------
int init_udp_socket();
int Receive_response();
void set_time_out(unsigned char);
unsigned char validate_response_header(unsigned char *,int);
unsigned char validate_resp_body_report_changes(unsigned int,int *,int *);
unsigned char validate_resp_body_get_page(unsigned int,int *,int *);
void Send_Request(unsigned int);
int prepare_send_req_header(unsigned char);
void Call_Report_Changes_Service();
unsigned char Process_response_Report_Changes();
void Call_Mirror_Get_Page_Service(unsigned int); 
unsigned char Process_response_Get_Page();
void Update_File_Contents(char [] , unsigned int, unsigned int);

#endif
