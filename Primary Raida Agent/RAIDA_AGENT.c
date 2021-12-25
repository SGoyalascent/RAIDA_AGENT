// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"


char execpath[256];
char Agent_Mode[10];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct timestamp tm;

//-------------------------------------------------
//Get the Working Directory
//------------------------------------------------
void get_execpath() {
    
    strcpy(execpath, "/opt/Testing/");
}
//-----------------------------------------------
// Welcome Message
//-----------------------------------------------
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
//-----------------------------------------------
//GET LASTEST TIMESTAMP
//-----------------------------------------------
void get_latest_timestamp(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    char datestring[256];
    time_t t1 = 0, t2;
    double time_dif;
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char f_path[500], f_name[256];
        sprintf(f_name, "%s",dir->d_name);
        sprintf(f_path, "%s/%s", path, dir->d_name);

        if((stat(f_path, &statbuf)) == -1) {
            fprintf(stderr,"Error: %s\n", strerror(errno));
            continue;
        }
        //if regular file
        if((statbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("filename: %s  filepath: %s\n", f_name, f_path);
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s\n", datestring);

            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
                t1 = t2;
                printf("datestring: %s  ", datestring);

                tm.year = dt->tm_year ;  //year from 1900 ==>  2021 == 121
                tm.month = dt->tm_mon;  //month in 0 - 11 range  ==> 12(dec) == 11
                tm.day = dt->tm_mday;
                tm.hour = dt->tm_hour;
                tm.minutes = dt->tm_min;
                tm.second = dt->tm_sec;
                printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month+1,tm.year+1900, tm.hour, tm.minutes, tm.second);
                //printf("Last Modified Time2:- %d-%d-%d  %d:%d:%d\n",dt->tm_mday,dt->tm_mon+1,dt->tm_year+1900, dt->tm_hour,dt->tm_min, dt->tm_sec);
            }
        }
        //if directory
        else if(((statbuf.st_mode & S_IFMT) == S_IFDIR) && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0) {
            printf("dirname: %s  dirpath: %s\n", f_name, f_path);
            show_dir_content(f_path);
        }
    }
    closedir(d);
}

int main() {

    WelcomeMsg();
    get_execpath();
    Read_Agent_Configuration_Files();
    Read_Keys();

    char *path;
    strcpy(path, execpath);
    strcat(path, "Data");
    get_latest_timestamp(path);

    /*
    int stat;
    if((stat = strcmp(Agent_Mode, "primary")) == 0){

    }
    else if((stat = strcmp(Agent_Mode, "mirror"))) {

    }
    else if((stat = strcmp(Agent_Mode, "witness"))) {

    }*/

    //Assume Primary RAIDA AGENT

    unsigned char status_code;
    Call_Report_Changes_Service();
    status_code = Process_response_Report_Changes();

    printf("Mirror Report Changes---Status_Code: %s\n", status_code);
    if(status_code == MIRROR_REPORT_RETURNED) {
        for() {
            Call_Mirror_Get_Page_Service();
            Process_response_Get_Page();
        }
    }

    return 0;

}

