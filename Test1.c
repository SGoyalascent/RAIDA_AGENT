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
#include <dirent.h>
#include <errno.h>
#include <stdint.h>


union bytes {
    unsigned int val;
    unsigned char byte[2];
};
union bytes binary;

void Writr_Binary_File() {

    FILE *fp;
    char path[256];
	unsigned char buff[4];
    unsigned int *port_number = 18000;
    unsigned int *bytes_per_frame = 1024;

    strcpy(path,"/opt/RAIDA_AGENT/server.bin");
    if ((fp = fopen(path, "wb")) == NULL) {
		printf("server.bin Cannot be opened , exiting \n");
		return;
	}

    fwrite(port_number, 2,1,fp);
	fwrite(bytes_per_frame, 2, 1, fp);

	/*
	binary.val = 18000;
	strcpy(buff, binary.byte);
	//buff[0] = binary.byte[0];
	//buff[1] = binary.byte[1];
	binary.val = 1024;
	strcat(buff, binary.byte);
	//buff[2] = binary.byte[0];
	//buff[3] = binary.byte[1];

	fwrite(buff 1, 4, fp); */

	fclose(fp);
}

int load_server_config() {
	FILE *fp_inp = NULL;
	int cnt=0;
	unsigned char buff[4];
	char path[256];
    unsigned int port_number, bytes_per_frame;
	strcpy(path,"/opt/RAIDA_AGENT/server.bin");
	if ((fp_inp = fopen(path, "rb")) == NULL) {
		printf("server.bin Cannot be opened , exiting \n");
		return 1;
	}
	if(fread(buff, 1, 4, fp_inp)<4){
		printf("Configuration parameters missing in server.bin \n");
		return 1;
	}
	
    /*
	port_number = buff[1];
	port_number|= (((uint16_t)buff[0])<<8);
	//server_config_obj.port_number = 18000;
	
	bytes_per_frame = buff[3];
	bytes_per_frame |= (((uint16_t)buff[2])<<8);
	//server_config_obj.bytes_per_frame = 1024;
	*/

	binary.byte[0] = buff[0];
    binary.byte[1] = buff[1];
    port_number = binary.val;
    binary.byte[0] = buff[2];
    binary.byte[1] = buff[3];
    bytes_per_frame = binary.val;

	printf("Port Number :- %d \n", port_number);
	printf("Bytes per UDP Request body :- %d \n", bytes_per_frame);
	fclose(fp_inp);
	return 0;
}

int main() {

    load_server_config();
}