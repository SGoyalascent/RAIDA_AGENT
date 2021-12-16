// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"

//-------------------

char execpath[256];
unsigned char send_buffer[MAXLINE];
unsigned char recv_buffer[MAXLINE];

struct timestamp tm;
struct sockaddr_in servaddr, cliaddr;
int sockfd = 0;

//-----------------------------------------------------------
//Initialize UDP Socket to send the Request
//-----------------------------------------------------------
int init_udp_socket() {

    int port_number;
	printf("init_udp_socket\n");
	if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
	        perror("socket creation failed");
	        exit(EXIT_FAILURE);
	    }
    else {
        printf("Socket Creation Successful\n");
    }
	memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_number);
    servaddr.sin_addr.s_addr = inet_addr("Mirror_ip_address");

    if ( bind(sockfd, (const struct sockaddr *)&servaddr,sizeof(servaddr)) < 0 ){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

    return 1;

}

//Send request to the Mirror Report Changes Service
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

//Get the Last Modified Time of the files in the  Directory
void show_dir_content(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    char datestring[256];
    struct tm *dt;
    time_t t1 = 0, t2;
    double time_dif;
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
            sprintf(filename, "%s",dir->d_name);
            sprintf(f_path, "%s/%s", path, dir->d_name);
            printf("filename: %s", filename);
            //printf("  filepath: %s\n", f_path);

            if(stat(f_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            strftime(datestring, sizeof(datestring), " %x-%X", tm);
            printf("datestring: %s\n", datestring);

            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
                t1 = t2;
                printf("datestring: %s  ", datestring);

                tm.year = dt->tm_year - 100;
                tm.month = dt->tm_mon;
                tm.day = dt->tm_mday;
                tm.hour = dt->tm_hour;
                tm.minutes = dt->tm_min;
                tm.second = dt->tm_sec;
                printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month,tm.year, tm.hour, tm.minutes, tm.second);
                //printf("Last Modified Time2:- %d-%d-%d  %d:%d:%d\n",dt->tm_mday,dt->tm_mon,dt->tm_year+1900, dt->tm_hour,dt->tm_min, dt->tm_sec);
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

void Receive_response_Report_Changes() {
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





int main() {

    //char path[256];
    //strcpy(execpath, "/opt/RAIDA_AGENT/Testing/raida/Data");
	//strcpy(path,execpath);
	//strcat(path,"/Data/...");
    //strcat(path, "/coin_0/ANs/1.bin");
    //strcpy(path, "/opt/RAIDA_AGENT/Testing/1.bin");

    char *path = "/opt/Testing/Data";
    show_dir_content(path);

    return 0;

}