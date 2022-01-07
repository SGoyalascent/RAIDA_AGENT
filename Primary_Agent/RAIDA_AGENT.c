// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"

char execpath[256], serverpath[256];
char Agent_Mode[10];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct timestamp tm;


//-----------------------------------------------
// Welcome Message
//-----------------------------------------------
void WelcomeMsg() {
    printf("\nWelcome to the RAIDA AGENT\n");
}
//-------------------------------------------------
//Get the Working Directory
//------------------------------------------------
void get_execpath() {
    
    strcpy(execpath, "/opt/Testing/");
    printf("execpath: %s\n", execpath);
}
//---------------------------------------------------------
// Get the current directory path starting from home dir
//---------------------------------------------------------
void getexepath()
{
  char buff[256];
  int count = readlink( "/proc/self/exe", buff, 256);
  int i=0,slash_pos;
  while(buff[i]!='\0'){
	if(buff[i]=='/'){
		slash_pos = i;
	}
	i++;
  }	
  strncpy(serverpath,buff,slash_pos);
}
//----------------------------------------------------------
//Loads raida no from raida_no.txt
//----------------------------------------------------------
int load_raida_no(){
	FILE *fp_inp=NULL;
	int size=0,ch;
	unsigned char buff[24];
	char path[256];
	strcpy(path,serverpath);
	strcat(path,"/Data/raida_no.txt");
	if ((fp_inp = fopen(path, "r")) == NULL) {
		printf("raida_no.txt Cannot be opened , exiting \n");
		return 1;
	}
	while( ( ch = fgetc(fp_inp) ) != EOF ){
		size++;
	}
	fclose(fp_inp);
	fp_inp = fopen(path, "r");
	if(fread(buff, 1, size, fp_inp)<size){
		printf("Configuration parameters missing in raida_no.txt \n");
		return 1;
	}
	if(size == 2){
		server_config_obj.raida_id = (buff[0]-48)*10;
		server_config_obj.raida_id+= (buff[1]-48);
	}else{
		server_config_obj.raida_id=buff[0]-48;
	}

    server_config_obj.bytes_per_frame = 1024;

	printf("Raida Id  :-%d Bytes_per_frame: %d\n", server_config_obj.raida_id, server_config_obj.bytes_per_frame);
	fclose(fp_inp);
	return 0;
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
        printf("Error. Can't find directory path\n");  
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

    printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    get_execpath();
    printf("-->MAIN: READ-Agent-Configuration-Files---\n");
    Read_Agent_Configuration_Files();
    //Read_Keys();

    char *path;
    strcpy(path, execpath);
    strcat(path, "Data");
    printf("-->MAIN: path: %s\n", path);
    printf("-->MAIN: GET-LATEST-TIMESTAMP---\n");
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

    init_udp_socket();

    unsigned char status_code;
    printf("-->MAIN: CALL-REPORT-CHANGES-SERVICE-----\n");
    Call_Report_Changes_Service();
    printf("-->MAIN: Call Process-Response-Report-Changes---\n");
    status_code = Process_response_Report_Changes();

    printf("-->MAIN: Report Changes---Status_Code: %s\n", status_code);
    if(status_code == MIRROR_REPORT_RETURNED) {
        for() {
            printf("MAIN: CALL- GET-page-service\n");
            Call_Mirror_Get_Page_Service();
            printf("MAIN: Process-Get-page\n");
            Process_response_Get_Page();
        }
    }

    return 0;

}

