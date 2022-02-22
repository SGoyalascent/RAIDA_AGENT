
#include "RAIDA_Agent.h"

char execpath[256], serverpath[256];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct timestamp tm;
struct server_config server_config_obj;
struct key_table key_table_obj[ENCRY2_KEYS_MAX] = {0};
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
		printf("Error: Configuration parameters missing in raida_no.txt \n");
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
int Read_Agent_Configuration_Files() {

    char path[256];
    strcpy(path, serverpath);
    strcat(path, "/Data/agent_config.txt");
    //printf("path: %s\n", path);
    FILE *myfile = fopen(path, "r");
    if(myfile == NULL) {
        perror("Error: agent_config.txt file not found\n");
		return 1;
    }
    fscanf(myfile, "ip_primary = %255s port_primary_agent = %d ip_mirror = %255s port_mirror_agent = %d ip_witness = %255s port_witness_agent = %d", 
    Primary_agent_config.Ip_address, &Primary_agent_config.port_number, Mirror_agent_config.Ip_address , 
    &Mirror_agent_config.port_number, Witness_agent_config.Ip_address, &Witness_agent_config.port_number);

    fclose(myfile);
    
    printf("ip_primary = %s  port_primary_agent = %d  ip_mirror = %s  port_mirror_agent = %d  ip_witness = %s  port_witness_agent = %d\n", Primary_agent_config.Ip_address, Primary_agent_config.port_number, Mirror_agent_config.Ip_address , Mirror_agent_config.port_number, Witness_agent_config.Ip_address, Witness_agent_config.port_number);

    return 0;
}

//-----------------------------------------------
//READ KEYS.bin FILE and store in the RAM
//-----------------------------------------------
int read_keys_file() {
    
    FILE *fp = NULL;
    int size = 0, ch;
    char path[500];
    char buff[KEY_BYTES_CNT*ENCRY2_KEYS_MAX];
    strcpy(path, serverpath);
    strcat(path, "/Keys/keys.bin");
    //printf("path: %s\n", path);
    if((fp = fopen(path, "rb")) == NULL) {
        perror("Error: Keys.bin file cannot be opened\n");
        return 1;
    }
    while((ch = fgetc(fp)) != EOF) {
        size++;
    }
    printf("Keys_file_size: %d\n", size);
    if(size != KEY_BYTES_CNT*ENCRY2_KEYS_MAX) {
        printf("Error: Keys file size does not match. Keys missing\n");
        return 1;
    }
    fclose(fp);

    fp = fopen(path, "rb");
    if(fread(buff, 1, size, fp) < size) {
        printf("Error: Keys bytes missing\n");
        return 1;
    }
    fclose(fp);
    
    int index = 0;
    for(int i = 0;i < ENCRY2_KEYS_MAX;i++) {
        printf("Key_Id-%d: ", i+1);
        memcpy(key_table_obj[i].key, &buff[index], KEY_BYTES_CNT);
        key_table_obj[i].key_id = i+1;
        index += KEY_BYTES_CNT;
        for(int j = 0; j < KEY_BYTES_CNT; j++) {
            printf("%hhn ", key_table_obj[i].key);
        }
        printf("\n");
    }
    return 0;
}
//-----------------------------------------------
//GET LASTEST TIMESTAMP
//-----------------------------------------------
int get_latest_timestamp(char *path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    time_t t2;
    double time_dif;

    int root_path_len = strlen(execpath);
    DIR *d = opendir(path); 
    if(d == NULL) {
        printf("Error: %s\n", strerror(errno));
        return 1;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char f_path[500], f_name[256], datestring[256], sub_path[500];
        sprintf(f_name, "%s",dir->d_name);
        sprintf(f_path, "%s/%s", path, dir->d_name);

        //if regular file
        if(dir->d_type == DT_REG) {
            //printf("\nfilename: %s  filepath: %s\n", f_name, f_path);
            if((stat(f_path, &statbuf)) == -1) {
                printf("Error: %s\n", strerror(errno));
                continue;
            }
            strcpy(sub_path, &f_path[root_path_len+1]);
            //printf("sub_path: %s\n", sub_path);
            if(strcmp(sub_path, f_name) == 0) {
                continue;
            }
            
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;    //modified time  mtime
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            //printf("datestring: %s  ", datestring);
            time_dif = difftime(t2, t1);
            //printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
                t1 = t2;

                tm.year = dt->tm_year;  //year from 1900 ==>  2021 == 121
                tm.month = dt->tm_mon;  //month in 0 - 11 range  ==> 12(dec) == 11
                tm.day = dt->tm_mday;
                tm.hour = dt->tm_hour;
                tm.minutes = dt->tm_min;
                tm.second = dt->tm_sec;
                //printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month+1,tm.year+1900, tm.hour, tm.minutes, tm.second);
                //time_t t3 = mktime(dt);
	            //printf("t3: %ju\n", t3);
            }
        }
        //if directory
        else if((dir->d_type == DT_DIR) && (strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0)) {
            //printf("dirname: %s  dirpath: %s\n", f_name, f_path);
            get_latest_timestamp(f_path);
        }
    }
    closedir(d);
    return 0;
}

//----------------------------------------------------------
// Returns time in micro seconds
//----------------------------------------------------------
long get_time_cs()
{
    long            ms; // Microseconds
    time_t          s;  // Seconds
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e3); // Convert nanoseconds to microseconds
	//printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",(intmax_t)s, ms);
    return ms;	
}

int main() {

    printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    getcurrentpath();
    get_execpath();
    //read_keys_file();
    if(load_raida_no() || Read_Agent_Configuration_Files()) {
        exit(0);
    }

    unsigned int time_before, time_after, exec_time, status;
    char path[256];
    strcpy(path, execpath);
    
    time_before = get_time_cs();
    status = get_latest_timestamp(path);
    time_after = get_time_cs();
    exec_time = time_after - time_before;
    printf("get_TimeStamp_exec_time: %u microseconds\n", exec_time);
    if(status == 1) {
        printf("Error: Could not get the latest file timestamp\n");
        //exit(0);
    }

    init_udp_socket();
    Call_Report_Changes_Service();
    status = Process_response_Report_Changes();
    
    if(status == 1) {
        for(unsigned int i = 0; i < total_files_count;i++) {
            Call_Mirror_Get_Page_Service(i);
            Process_response_Get_Page();
        }
    }
    return 0;
}

