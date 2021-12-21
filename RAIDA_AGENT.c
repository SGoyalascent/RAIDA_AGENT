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
    char path[50] = "/opt/Testing/";
    int path_len = strlen(path);
    char Agent_Mode[10];
    char ip_address_Primary[20], ip_address_Mirror[20], ip_address_Witness[20];
    unsigned int port_primary, port_mirror, port_witness;

    struct dirent *dir; 
    DIR *d = opendir(path); 
    if(d == NULL) {
        pritnf("Error\n");  
    }
    int i=0;
    while ((dir = readdir(d)) != NULL) 
    {
        if(dir->d_type == DT_REG) {
            
			char f_path[500];
            char f_name[50];
            strcpy(f_name, dir->d_name);
            strcpy(f_path, path);
            strcat(f_path, f_name);
            printf("filename: %s  filepath: %s ", f_name, f_path);

            strcpy(file_name[i], f_name);
            printf("file_name: %s\n", file_name[i]);
            i++;

            char *token;
            char *token1;
            token = strtok(f_name, ".");
            while(token != NULL) {

                printf("%s ", token);
                strcpy(token1, token);
                token = strtok(NULL, ".");
            }
            printf("\n");
            printf("token1: %s\n", token1);

            if(strcmp(token1, "config") == 0) {
                token = strtok(f_name, ".");
                token = strtok(NULL, ".");
                strcpy(Agent_Mode, token);
                printf("Mode: %s ", Agent_Mode);
            }
            else if(strcmp(token1, "ip") == 0) {
                token = strtok(f_name, ".");
                if(strcmp(token, "primary") == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Primary, token);
                    token = strtok(NULL, ".");
                    port_primary = atoi(token);
                    printf("PRIMARY.  Ip_address: %s Port: %d\n", ip_address_Primary, port_primary);
                }
                else if(strcmp(token, "mirror") == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Mirror, token);
                    token = strtok(NULL, ".");
                    port_mirror = atoi(token);
                    printf("MIRROR.  Ip_address: %s Port: %d\n", ip_address_Mirror, port_mirror);
                }
                else if(strcmp(token, "witness") == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Witness, token);
                    token = strtok(NULL, ".");
                    port_witness = atoi(token);
                    printf("WITNESS.  Ip_address: %s Port: %d\n", ip_address_Witness, port_witness);
                }
            }

        }	
			
    }
    closedir(d);
}
