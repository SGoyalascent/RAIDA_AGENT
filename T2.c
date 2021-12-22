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
    unsigned int port_number = 18000;
    unsigned int bytes_per_frame = 1024;


//------------1------
    
	fwrite(&port_number, 2,1,fp);
	fwrite(&bytes_per_frame, 2, 1, fp); 

	port_number = buff[1];                              
	port_number|= (((uint16_t)buff[1])<<8);
	
	bytes_per_frame = buff[3];
	bytes_per_frame |= (((uint16_t)buff[2])<<8);

    printf("Port Number :- %d \n", port_number);   20550
	printf("Bytes per UDP Request body :- %d \n", bytes_per_frame);  4
    0000000 4650 0400 0000004

//----------2-------	
	
	
 
//-----------3------------
	
	binary.val = 18000;
	buff[0] = binary.byte[0];           
	buff[1] = binary.byte[1];
	printf("byte: %d %d  ", binary.byte[0], binary.byte[1]);   80  70

	binary.val = 1024;
	buff[2] = binary.byte[0];
	buff[3] = binary.byte[1];
	printf("byte: %d %d \n", binary.byte[0], binary.byte[1]);  0   4
	printf("buff: %d %d %d %d\n", buff[0], buff[1], buff[2], buff[3]); 80   70   0   4

	fwrite(buff, 1, 4, fp); 

	port_number = buff[1];                              
	port_number|= (((uint16_t)buff[0])<<8);
	
	bytes_per_frame = buff[3];
	bytes_per_frame |= (((uint16_t)buff[2])<<8);

    printf("Port Number :- %d \n", port_number);     20550
	printf("Bytes per UDP Request body :- %d \n", bytes_per_frame);   4
	0000000 4650 0400 0000004


//----------------5---------

//------------1------
    
	fwrite(&port_number, 2,1,fp);
	fwrite(&bytes_per_frame, 2, 1, fp); 

	binary.byte[0] = buff[0];
    binary.byte[1] = buff[1];                          
    port_number = binary.val;
    binary.byte[0] = buff[2];
    binary.byte[1] = buff[3];
    bytes_per_frame = binary.val; 

    printf("buff: %d %d %d %d\n", buff[0], buff[1], buff[2], buff[3]);   80   70   0   4

    printf("Port Number :- %d \n", port_number);  18000
	printf("Bytes per UDP Request body :- %d \n", bytes_per_frame); 1024
    0000000 4650 0400 0000004
    

//-----------3------------
	
	binary.val = 18000;
	buff[0] = binary.byte[0];           
	buff[1] = binary.byte[1];
	printf("byte: %d %d  ", binary.byte[0], binary.byte[1]);  80   70 

	binary.val = 1024;
	buff[2] = binary.byte[0];
	buff[3] = binary.byte[1];
	printf("byte: %d %d \n", binary.byte[0], binary.byte[1]);  0   4
	printf("buff: %d %d %d %d\n", buff[0], buff[1], buff[2], buff[3]);  80   70   0   4

	fwrite(buff, 1, 4, fp); 

    binary.byte[0] = buff[0];
    binary.byte[1] = buff[1];                          
    port_number = binary.val;
    binary.byte[0] = buff[2];
    binary.byte[1] = buff[3];
    bytes_per_frame = binary.val; 

    printf("buff: %d %d %d %d\n", buff[0], buff[1], buff[2], buff[3]);  80   70   0   4
	
    printf("Port Number :- %d \n", port_number);   18000
	printf("Bytes per UDP Request body :- %d \n", bytes_per_frame); 1024
    0000000 4650 0400 0000004
	

}

int main() {

	Writr_Binary_File();
    load_server_config();
}