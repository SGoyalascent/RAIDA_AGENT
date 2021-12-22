// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"


char execpath[256];
struct agent_config agent_config_obj;


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
                strcpy(agent_config_obj.Agent_Mode, token);
                printf("Agent_Mode: %s ", agent_config_obj.Agent_Mode);
            }
            else if((stat = strcmp(token1, "ip")) == 0) {
                token = strtok(name, ".");
                if((stat = strcmp(token, "primary")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(agent_config_obj.ip_address_Primary, token);
                    token = strtok(NULL, ".");
                    agent_config_obj.port_primary = atoi(token);
                    printf("PRIMARY-RAIDA.  Ip_address: %s Port: %d\n", agent_config_obj.ip_address_Primary, agent_config_obj.port_primary);
                }
                else if((stat = strcmp(token, "mirror")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(agent_config_obj.ip_address_Mirror, token);
                    token = strtok(NULL, ".");
                    agent_config_obj.port_mirror = atoi(token);
                    printf("MIRROR-RAIDA.  Ip_address: %s Port: %d\n", agent_config_obj.ip_address_Mirror, agent_config_obj.port_mirror);
                }
                else if((stat = strcmp(token, "witness")) == 0) {
                    token = strtok(NULL, ":");
                    strcpy(agent_config_obj.ip_address_Witness, token);
                    token = strtok(NULL, ".");
                    agent_config_obj.port_witness = atoi(token);
                    printf("WITNESS-RAIDA.  Ip_address: %s Port: %d\n",agent_config_obj.ip_address_Witness, agent_config_obj.port_witness);
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
    if((stat = strcmp(agent_config_obj.Agent_Mode, "primary")) == 0) {

    }
    else if((stat = strcmp(agent_config_obj.Agent_Mode, "mirror"))) {

    }
    else if((stat = strcmp(agent_config_obj.Agent_Mode, "witness"))) {

    }

    return 0;

}

