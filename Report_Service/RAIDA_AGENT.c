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
    //printf("Working_Dir_path: %s\n", execpath);
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
  //printf("Current_dir_path: %s\n", serverpath);
}
//----------------------------------------------------------
//Loads raida no from raida_no.txt
//----------------------------------------------------------
int load_raida_no(){
	
    FILE *fp_inp=NULL;
	int size=0,ch;
	unsigned char buff[24];
	char path[256];
	strcpy(path,execpath);
	strcat(path,"/raida_no.txt");
    //printf("path: %s\n", path);
	if ((fp_inp = fopen(path, "r")) == NULL) {
        perror("Error: raida_no.txt cannot be opened.");
		return 1;
	}
	while( (ch = fgetc(fp_inp)) != EOF ){
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

    char path[256];
    strcpy(path, serverpath);
    strcat(path, "/Data_agent/agent_config.txt");
    //printf("path: %s\n", path);
    FILE *myfile = fopen(path, "r");
    if(myfile == NULL) {
        perror("Error: agent_config.txt file not found\n");
		return;
    }
    fscanf(myfile, "ip_primary = %255s port_primary_agent = %d ip_mirror = %255s port_mirror_agent = %d ip_witness = %255s port_witness_agent = %d", 
    Primary_agent_config.Ip_address, &Primary_agent_config.port_number, Mirror_agent_config.Ip_address , 
    &Mirror_agent_config.port_number, Witness_agent_config.Ip_address, &Witness_agent_config.port_number);

    fclose(myfile);
    
    printf("ip_primary = %s  port_primary_agent = %d  ip_mirror = %s  port_mirror_agent = %d  ip_witness = %s  port_witness_agent = %d\n", 
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
        perror("Error: Keys.bin file cannot be opened\n");
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

    int root_path_len = strlen(execpath);
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char f_path[500], f_name[256];
        sprintf(f_name, "%s",dir->d_name);
        sprintf(f_path, "%s/%s", path, dir->d_name);

       
        //if regular file
        if((statbuf.st_mode & S_IFMT) == S_IFREG) {
            printf("\nfilename: %s  filepath: %s\n", f_name, f_path);
            
            if((stat(f_path, &statbuf)) == -1) {
            printf("Error: %s\n", strerror(errno));
            continue;
            }
            char sub_path[500];
            strcpy(sub_path, &f_path[root_path_len+1]);
            //printf("sub_path: %s\n", sub_path);
            if(strcmp(sub_path, f_name) == 0) {
                continue;
            }
            
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;    //modified time  mtime
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s  ", datestring);

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
            //printf("dirname: %s  dirpath: %s\n", f_name, f_path);
            get_latest_timestamp(f_path);
        }
    }
    closedir(d);
}

//----------------------------------------------------------
// Returns time in centi seconds
//----------------------------------------------------------
/*
long get_time_cs()
{
    long            ms,cs; // Microseconds
    time_t          s;  // Seconds
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e3); // Convert nanoseconds to microseconds
    //cs = ms /100;	
	//printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",(intmax_t)s, ms);
    return ms;	
}
*/

int main() {

    printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    getcurrentpath();
    get_execpath();
    load_raida_no();
    Read_Agent_Configuration_Files();
    //read_keys_file();

    char path[256];
    strcpy(path, execpath);
    //printf("path: %s\n", path);
    printf("----GET LATEST TIMESTAMP----\n");
    get_latest_timestamp(path);
    printf("latest_file_time: %ju", t1);

    init_udp_socket();
    unsigned char status;
    Call_Report_Changes_Service();
    status = Process_response_Report_Changes();
    
    //Call_Mirror_Get_Page_Service(0);
    //Process_response_Get_Page();
    
    if(status == 1) {
        /*
        for(unsigned int i = 0; i < total_files_count;i++) {
            printf("MAIN: CALL- GET-page-service\n");
            Call_Mirror_Get_Page_Service(i);
            printf("MAIN: Process-Get-page\n");
            Process_response_Get_Page();
        }
        */
    }
    return 0;

}

