#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

time_t t1;
unsigned int coin_id;
unsigned int table_id;
unsigned int serial_no;

void show_dir_content(char *path)
{
    struct dirent *dir; 
    struct stat statbuf;
    struct tm *dt;
    
	char root_path[256] = "/opt/Testing/Data";
    int path_len = strlen(root_path);

    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        char df_path[500];
        char df_name[256];
        sprintf(df_name, "%s",dir->d_name);
        sprintf(df_path, "%s/%s", path, dir->d_name);

        if(dir->d_type == DT_REG) {
            
			printf("filename: %s  filepath: %s\n", df_name, df_path);

            //Compare timestamps
            time_t t2;
            char datestring[256];
            double time_dif;
            char sub_path[500];

            if(stat(df_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }

            strcpy(sub_path, df_path+path_len+1);
            printf("sub_path: %s\n", sub_path);

            if(strcmp(sub_path, df_name) == 0) {
                continue;
            }

            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            //strftime(datestring, sizeof(datestring), " %x-%X", dt);
            //printf("datestring: %s\n", datestring);
            printf("Last Modified Time(UTC):- %d-%d-%d  %d:%d:%d\n", dt->tm_mday,dt->tm_mon,dt->tm_year+1900, 
                                                                                dt->tm_hour,dt->tm_min, dt->tm_sec );
        
            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif <= 0) {
                printf("File already Syncronized.\n");
                continue;
            }
            printf("File Modified. Need to be Syncronized.\n");
            
            char coin[20];
            char table[20];
            unsigned char c_id[10];

            char *token;
            token = strtok(sub_path, "/");
            strcpy(coin, token);

            token = strtok(NULL, "/");
            strcpy(table, token);

            //token = strtok(NULL, "/");
            //strcpy(serial, token);

            token = strtok(coin, "_");
            token = strtok(NULL,"_");
            strcpy(c_id, token);
            //printf("c_id: %s  ", c_id);
            coin_id = atoi(c_id);
            serial_no = atoi(df_name);

            printf("coin_id: %d  table: %s  serial_no: %d\n", coin_id, table, serial_no);

            if(strcmp(table, "ANs") == 0) {
                table_id = 0;
            }
            else if(strcmp(table, "Owners") == 0) {
                table_id = 1;
            }
            else if(strcmp(table, "Statements") == 0) {
                table_id = 2;
            }
            else if(strcmp(table, "Loss_Coin_Report") == 0) {
                table_id = 3;
            }
            else if(strcmp(table, "Email_Recover") == 0) {
                table_id = 4;
            }
        
            printf("coin_id: %d  table_id: %d  serial_no: %d\n", coin_id, table_id, serial_no);
            continue;
        }	
			
        if((dir->d_type == DT_DIR) && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("dir_name: %s  dir_path: %s\n", df_name,df_path);
            show_dir_content(df_path); 
        }
    }
    closedir(d);
}

int main() {

    char *path = "/opt/Testing/Data";
    show_dir_content(path);
    return(0);
}