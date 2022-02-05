// When in Primary mode, Call the Mirror Report Changes service
// Send request to the Mirror server, receive the response send from the mirror
// Assume we are using Standard 22 bytes Header

#include "RAIDA_Agent.h"

char execpath[256], serverpath[256], keys_bytes[KEYS_COUNT][KEY_BYTES_CNT];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct timestamp tm;
struct server_config server_config_obj;
time_t t1 = 0;

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
    strcpy(execpath, "/opt/raida/Data");
    //strcpy(execpath, "/mnt/e/GIT REPOS/Goyal_ASCENT/RAIDA_AGENT/Testing/raida/Data");   //for testing only
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
	
    //printf("-->Load RAIDA No.\n");
    FILE *fp_inp=NULL;
	int size=0,ch;
	unsigned char buff[24];
	char path[256];
	strcpy(path,serverpath);
	strcat(path,"/Data_agent/raida_no.txt");
    printf("path: %s\n", path);
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

	printf("Raida Id  :-%d   Bytes_per_frame: %d\n", server_config_obj.raida_id, server_config_obj.bytes_per_frame);
	fclose(fp_inp);
	return 0;
}	

//--------------------------------------------------
//READ CONFIG FILE, IP ADDRESS AND PORT
//--------------------------------------------------
void Read_Agent_Configuration_Files() {

    //printf("-->READ-Agent-Configuration-Files---\n");
    char path[256];
    strcpy(path, serverpath);
    strcat(path, "/Data_agent/agent_config.txt");
    printf("path: %s\n", path);
    FILE *myfile = fopen(path, "r");
    if(myfile == NULL) {
        printf("agent_config file not found\n");
		return;
    }
    fscanf(myfile, "ip_primary = %255s port_primary_agent = %d ip_mirror = %255s port_mirror_agent = %d ip_witness = %255s port_witness_agent = %d", 
    Primary_agent_config.Ip_address, &Primary_agent_config.port_number, Mirror_agent_config.Ip_address , 
    &Mirror_agent_config.port_number, Witness_agent_config.Ip_address, &Witness_agent_config.port_number);

    fclose(myfile);
    
    printf("ip_primary = %s\t\t port_primary_agent = %d \t\t ip_mirror = %s\t\t port_mirror_agent = %d\t\t ip_witness = %s\t\t port_witness_agent = %d\n", 
    Primary_agent_config.Ip_address, Primary_agent_config.port_number, Mirror_agent_config.Ip_address , 
    Mirror_agent_config.port_number, Witness_agent_config.Ip_address, Witness_agent_config.port_number);

}

//-----------------------------------------------
//READ KEYS.bin FILE and store in the RAM
//-----------------------------------------------
void read_keys_file() {
    
    FILE *fp = NULL;
    int size = 0, ch;
    char path[500];
    char buff[KEY_BYTES_CNT*KEYS_COUNT];
    strcpy(path, serverpath);
    strcat(path, "/Keys/keys.bin");
    printf("path: %s\n", path);
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
void get_latest_timestamp(char *path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    char datestring[256];
    time_t t2;
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
            printf("Error: %s\n", strerror(errno));
            continue;
        }
        //if regular file
        if((statbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("filename: %s  filepath: %s\n", f_name, f_path);
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;    //modified time  mtime
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s\n", datestring);

            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
                t1 = t2;

                tm.year = dt->tm_year;  //year from 1900 ==>  2021 == 121
                tm.month = dt->tm_mon;  //month in 0 - 11 range  ==> 12(dec) == 11
                tm.day = dt->tm_mday;
                tm.hour = dt->tm_hour;
                tm.minutes = dt->tm_min;
                tm.second = dt->tm_sec;
                printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month+1,tm.year+1900, tm.hour, tm.minutes, tm.second);
            }
        }
        //if directory
        else if(((statbuf.st_mode & S_IFMT) == S_IFDIR) && (strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0)) {
            printf("dirname: %s  dirpath: %s\n", f_name, f_path);
            get_latest_timestamp(f_path);
        }
    }
    closedir(d);
}

int main() {

    //printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    getcurrentpath();
    get_execpath();
    load_raida_no();
    Read_Agent_Configuration_Files();
    read_keys_file();

    char path[256];
    strcpy(path, execpath);
    printf("path: %s\n", path);
    printf("-->MAIN: GET-LATEST-TIMESTAMP---\n");
    get_latest_timestamp(path);

    init_udp_socket();

    unsigned char status_code;
    Call_Report_Changes_Service();
    status_code = Process_response_Report_Changes();
    printf("-->MAIN: Report Changes---Status_Code: %d\n", status_code);
    if(status_code == FAIL) {
        printf("Mirror Report Changes Service could not be called\n");
    } 
    else if(status_code == RAIDA_AGENT_NO_CHANGES) {
        printf("No need to Call GET Page service\n");
    }
    else if(status_code == MIRROR_REPORT_RETURNED) {
        for(unsigned int i = 0; i < total_files_count;i++) {
            printf("MAIN: CALL- GET-page-service\n");
            Call_Mirror_Get_Page_Service(i);
            printf("MAIN: Process-Get-page\n");
            Process_response_Get_Page();
        }
    }
    return 0;

}

