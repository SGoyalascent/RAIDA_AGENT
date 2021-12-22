// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"


char execpath[256];
unsigned char send_buffer[MAXLINE], recv_buffer[MAXLINE];

char Agent_Mode[10];
char ip_address_Primary[20], ip_address_Mirror[20], ip_address_Witness[20];
unsigned int port_primary, port_mirror, port_witness;


void Read_Agent_Configuration_Files() {

    char path[50] = "/opt/Testing/";
    
    struct dirent *dir; 
    DIR *d = opendir(path); 
    if(d == NULL) {
        printf("Error\n");  
    }
    int i=0;
    while ((dir = readdir(d)) != NULL) 
    {
        if(dir->d_type == DT_REG) {
            
			char f_path[500], f_name[50];
            strcpy(f_name, dir->d_name);
            strcpy(f_path, path);
            strcat(f_path, f_name);
            printf("filename: %s  filepath: %s\n", f_name, f_path);

            char *token;
            char *token1;
            char name[50];
            strcpy(name, dir->d_name);
            token = strtok(name, ".");
            while(token != NULL) {

                strcpy(token1, token);
                token = strtok(NULL, ".");
            }

            strcpy(name, dir->d_name);
            int stat;
            if((stat = strcmp(token1, "config")) == 0) {
                
                token = strtok(name, ".");
                token = strtok(NULL, ".");
                strcpy(Agent_Mode, token);
                printf("Agent_Mode: %s ", Agent_Mode);
            }
            else if((stat = strcmp(token1, "ip")) == 0) {
                token = strtok(name, ".");
                if((stat = strcmp(token, "primary")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Primary, token);
                    token = strtok(NULL, ".");
                    port_primary = atoi(token);
                    printf("PRIMARY-RAIDA.  Ip_address: %s Port: %d\n", ip_address_Primary, port_primary);
                }
                else if((stat = strcmp(token, "mirror")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Mirror, token);
                    token = strtok(NULL, ".");
                    port_mirror = atoi(token);
                    printf("MIRROR-RAIDA.  Ip_address: %s Port: %d\n", ip_address_Mirror, port_mirror);
                }
                else if((stat = strcmp(token, "witness")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(ip_address_Witness, token);
                    token = strtok(NULL, ".");
                    port_witness = atoi(token);
                    printf("WITNESS-RAIDA.  Ip_address: %s Port: %d\n", ip_address_Witness, port_witness);
                }
            } 

        }	
			
    }
    closedir(d);
}