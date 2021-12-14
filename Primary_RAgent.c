// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

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
#include <time.h>


#define VER 255
#define PORT     18000
#define MAXLINE 1024
#define REQUEST_HEADER  22
#define CH_BYTES_CNT				16
#define TIMESTAMP_BYTES_CNT   6

union time {
    unsigned int val;
    unsigned char byte[1];
};

union time ts;

char execpath[256];

struct timestamp {

    unsigned char year;
    unsigned char month[1];
    unsigned char day[1];
    unsigned char hour[1];
    unsigned char minutes[1];
    unsigned char second[1];
};

struct timestamp tm;


//Send request to the Mirror Report Changes Service
void Call_ReportChanges_Mirror() {

    unsigned char request_header[REQUEST_HEADER] = {0,0,2,0,0,215,0,0,0,0,0,0,22,22,0,1,0,0,0,0,0,0};
    unsigned char buffer[MAXLINE];

    memcpy(buffer,request_header,REQUEST_HEADER);

    for(int i=0; i < CH_BYTES_CNT; i++) {

        buffer[REQUEST_HEADER +i ] = i+1;
    }

    //ts.val = tm.year - 2000;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT] = tm.year - 2000;

    //ts.val = tm.month;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT + 1] = tm.month[0];

    ts.val = tm.day;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT + 2] = ts.byte[0];

    ts.val = tm.hour;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT + 3] = ts.byte[0];

    ts.val = tm.minutes;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT + 4] = ts.byte[0];

    ts.val = tm.second;
    //printf("%d %d\n", ts.val, ts.byte[0]);
    buffer[REQUEST_HEADER + CH_BYTES_CNT + 5] = ts.byte[0];

    printf("send_buffer:- ");

    for(int i=0; i < REQUEST_HEADER+CH_BYTES_CNT+TIMESTAMP_BYTES_CNT; i++) {

        printf("%d ", buffer[i] );
    }
    printf("\n");



}


void getLastModifiedTime() {

    char path[256];
    //strcpy(execpath, "/opt/RAIDA_AGENT/Testing/raida/Data");
	//strcpy(path,execpath);
	//strcat(path,"/Data/...");
    //strcat(path, "/coin_0/ANs/1.bin");
    strcpy(path, "/opt/RAIDA_AGENT/Testing/1.bin");
    
    struct stat attrib;
    int status = stat(path, &attrib);

    if(status != 0) {
        printf("error\n");
        return;
    }

    struct tm *dt;

    dt = localtime(&attrib.st_mtime);

    tm.year = dt->tm_year + 1900;
    tm.month = dt->tm_mon;
    tm.day = dt->tm_mday;
    tm.hour = dt->tm_hour;
    tm.minutes = dt->tm_min;
    tm.second = dt->tm_sec;
    printf("local time: ");
    //printf("Last Modified Time2:- %d-%d-%d  %d:%d:%d\n",dt->tm_mday,dt->tm_mon,dt->tm_year+1900, dt->tm_hour,dt->tm_min, dt->tm_sec);
    printf("Last Modified Time3:- %d-%d-%d  %d:%d:%d\n",tm.day, tm.month,tm.year, tm.hour, tm.minutes, tm.second);

    Call_ReportChanges_Mirror();

}

int main() {

    //Call_ReportChanges_Mirror();
    getLastModifiedTime();

    return 0;

}