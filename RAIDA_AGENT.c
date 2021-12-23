// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"


char execpath[256];
char Agent_Mode[10];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;


void get_execpath() {
    
    strcpy(execpath, "/opt/Testing/");
}

void WelcomeMsg() {
    printf("\nWelcome to the RAIDA AGENT\n");
}

//--------------------------------------------------
//READ CONFIG FILE, IP ADDRESS AND PORT
//--------------------------------------------------
void Read_Agent_Configuration_Files() {

    char path[50];
    strcpy(path, execpath);
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
                    strcpy(Primary_agent_config.Ip_address, token);
                    token = strtok(NULL, ".");
                    Primary_agent_config.port_number = atoi(token);
                    printf("PRIMARY-RAIDA.  Ip_address: %s Port: %d\n", Primary_agent_config.Ip_address, Primary_agent_config.port_number);
                }
                else if((stat = strcmp(token, "mirror")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(Mirror_agent_config.Ip_address, token);
                    token = strtok(NULL, ".");
                    Mirror_agent_config.port_number = atoi(token);
                    printf("MIRROR-RAIDA.  Ip_address: %s Port: %d\n", Mirror_agent_config.Ip_address, Mirror_agent_config.port_number);
                }
                else if((stat = strcmp(token, "witness")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(Witness_agent_config.Ip_address, token);
                    token = strtok(NULL, ".");
                    Witness_agent_config.port_number = atoi(token);
                    printf("WITNESS-RAIDA.  Ip_address: %s Port: %d\n", Witness_agent_config.Ip_address, Witness_agent_config.port_number);
                }
            } 

        }	
			
    }
    closedir(d);
}

//-----------------------------------------------
//READ KEYS.bin FILE
//-----------------------------------------------

void Read_Keys() {

}

int main() {

    WelcomeMsg();
    get_execpath();
    Read_Agent_Configuration_Files();
    Read_Keys();

    int stat;
    if((stat = strcmp(Agent_Mode, "primary")) == 0) {

    }
    else if((stat = strcmp(Agent_Mode, "mirror"))) {

    }
    else if((stat = strcmp(Agent_Mode, "witness"))) {

    }

    return 0;

}

