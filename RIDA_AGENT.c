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




int main() {

    char file_name[6][50];
    char path[500] = "/opt/Testing";
    struct dirent *dir; 
    int path_len = strlen(path);
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    int i=0;
    while ((dir = readdir(d)) != NULL) 
    {
        if(dir->d_type == DT_REG) {
            
			char f_path[500];
            char f_name[256];
            sprintf(f_name, "%s",dir->d_name);
            sprintf(f_path, "%s/%s", path, dir->d_name);
            printf("filename: %s  filepath: %s ", f_name, f_path);

            strcpy(file_name[i], f_name);
            i++;
            printf("file_name: %s\n", file_name[i]);
            //char *token;
            //token = strtok(f_name, ".");

            //while(token != NULL) {

        }	
			
    }
    closedir(d);
}
