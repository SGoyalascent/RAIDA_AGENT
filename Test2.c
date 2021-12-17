// Mirror_Report_Changes---------Check Changes in the Directory

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

unsigned char resp_body[5000];

void show_dir_content(char* path) {
    struct dirent *dir; 
    struct stat statbuf;
    char datestring[256];
    struct tm *dt;
    time_t t2;
    double time_dif;
    char org_path[256] = "/opt/Testing/Data";
    int path_len = strlen(org_path);
    DIR *d = opendir(path); 
    if(d == NULL) {
        return;  
    }
    while ((dir = readdir(d)) != NULL) 
    {
        // if the type is not directory
        if(dir->d_type == DT_REG) {
            
            char f_path[500];
            char filename[256];
            char sub_path[500];
            sprintf(filename, "%s",dir->d_name);
            sprintf(f_path, "%s/%s", path, dir->d_name);
            printf("filename: %s  filepath: %s\n",filename, f_path);

            int file_len = strlen(filename);
            int f_path_len = strlen(f_path);

            strcpy(sub_path, f_path+path_len);
            printf("sub_path: %s\n", sub_path);


            
        }
        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            char d_path[500]; 
            char dir_name[256];
            sprintf(d_path, "%s/%s", path, dir->d_name);
            sprintf(dir_name, "%s", dir->d_name);
            printf("dir_name: %s  dir_path: %s\n",dir_name, d_path);
            /*
            int dir_len = strlen(dir_name);
            int res;

            strcmp(dir_name, "ANs");
            strcmp(dir_name, "Owners");
            strcmp(dir_name, "Statements");
            strcmp(dir_name, "Loss_Coin_Report");
            strcmp(dir_name, "Email_Recover");*/

            show_dir_content(d_path); 
        }
    }
    closedir(d); 
}
int main() {

    char *path = "/opt/Testing/Data";
    show_dir_content(path);
    return(0);
}