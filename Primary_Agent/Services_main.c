// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"

char execpath[256], serverpath[256], Agent_Mode[10], keys_bytes[KEYS_COUNT][KEY_BYTES_CNT];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct timestamp tm;
struct server_config server_config_obj;
time_t t1 = 0;

//-----------------------------------------------
// Welcome Message
//-----------------------------------------------
void WelcomeMsg() {
    printf("\nWelcome to the RAIDA Services\n");
}
//-------------------------------------------------
//Get the Working Directory
//------------------------------------------------
void get_execpath() {
    strcpy(execpath, "/opt/Testing");
    printf("Working_Dir_path: %s\n", execpath);
}
//---------------------------------------------------------
// Get the current directory path starting from home dir
//---------------------------------------------------------
void getcurrentpath()
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
  printf("Current_dir_path: %s\n", serverpath);
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
		printf("->Error: raida_no.txt Cannot be opened , exiting \n");
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
        printf("Error: Can't find directory path\n");  
    }
    int i=0;
    while ((dir = readdir(d)) != NULL) 
    {
        if(dir->d_type == DT_REG) {

			char f_path[500], f_name[50];
            strcpy(f_name, dir->d_name);
            strcpy(f_path, path);
            strcat(f_path, "/");
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
//READ KEYS.bin FILE and store in the RAM
//-----------------------------------------------
void read_keys_file() {
    
    FILE *fp = NULL;
    int size = 0, ch;
    char path[500];
    char buff[KEY_BYTES_CNT*KEYS_COUNT];
    strcpy(path, execpath);
    strcat(path, "/Keys/keys.bin");
    if((fp = fopen(path, "rb")) == NULL) {
        printf("->Error: Keys.bin file cannot be opened\n");
        return;
    }
    while((ch = fgetc(fp)) != EOF) {
        size++;
    }
    printf("Keys_file_size: %d\n", size);
    if(size != KEY_BYTES_CNT*KEYS_COUNT) {
        printf("Error: Keys file size does not match. Keys missing\n");
        return;
    }
    fclose(fp);

    fp = fopen(path, "rb");
    if(fread(buff, 1, size, fp) < size) {
        printf("Keys bytes missing\n");
        return;
    }
    fclose(fp);
    
    int index = 0;
    for(int i = 0;i < KEYS_COUNT;i++) {
        printf("KEY_%d: ", i+1);
        memcpy(&keys_bytes[i][0], &buff[index], KEY_BYTES_CNT);
        index += KEY_BYTES_CNT;
        for(int j = 0; j < KEY_BYTES_CNT; j++) {
            printf("%d ", keys_bytes[i][j]);
        }
        printf("\n");
    }

}
//-----------------------------------------------
//GET LASTEST TIMESTAMP
//-----------------------------------------------

int main() {

    printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    getcurrentpath();
    get_execpath();
    load_raida_no();
    printf("-->READ-Agent-Configuration-Files---\n");
    Read_Agent_Configuration_Files();
    read_keys_file();


    //Assume Primary RAIDA AGENT

    init_udp_socket();

    unsigned char status_code;
    Call_Report_Changes_Service();
    status_code = Process_response_Report_Changes();
    printf("-->MAIN: Report Changes---Status_Code: %d\n", status_code);
    if(status_code == FAIL) {
        printf("Again Call Report Changes Service\n");
        //Call_Report_Changes_Service();
        //status_code = Process_response_Report_Changes();
        //printf("-->MAIN: Report Changes---Status_Code: %d\n", status_code);
    } 
    if(status_code == RAIDA_AGENT_NO_CHANGES) {
        printf("No need to Call GET Page service\n");
    }
    
    if(status_code == MIRROR_REPORT_RETURNED) {
        for(unsigned int i = 0; i < total_files_count;i++) {
            printf("MAIN: CALL- GET-page-service\n");
            Call_Mirror_Get_Page_Service(i);
            printf("MAIN: Process-Get-page\n");
            Process_response_Get_Page();
        }
    }

    return 0;

}

