
#include "Agent_Services.h"

char execpath[256], serverpath[256], keys_bytes[KEYS_COUNT][KEY_BYTES_CNT];
struct agent_config Primary_agent_config, Mirror_agent_config, Witness_agent_config;
struct server_config server_config_obj;

//-----------------------------------------------
// Welcome Message
//-----------------------------------------------
void WelcomeMsg() {
    printf("\nWelcome to the RAIDA ServicesT\n");
}
//-------------------------------------------------
//Get the Working Directory
//------------------------------------------------
void get_execpath() {
    strcpy(execpath, "/opt/raida/Data");
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
	
    printf("-->Load RAIDA No.\n");
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

    printf("-->READ-Agent-Configuration-Files---\n");
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
//----------------------------------------------------------
// Returns time in centi seconds
//----------------------------------------------------------
long get_time_cs()
{
    long            ms,cs; // Microseconds
    time_t          s;  // Seconds
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e3); // Convert nanoseconds to microseconds  
    //cs = ms /100;	
	printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",(intmax_t)s, ms);
    return ms;	
}
int main() {

    printf("MAIN: ------------------------------------RAIDA-AGENT-MAIN-----------------------------------\n");
    WelcomeMsg();
    getcurrentpath();
    get_execpath();
    load_raida_no();
    Read_Agent_Configuration_Files();
    //read_keys_file();

    init_udp_socket();

    listen_request();

    return 0;

}

