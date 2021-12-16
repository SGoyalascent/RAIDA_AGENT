// Mirror_Report_Changes---------Check Changes in the Directory

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>


int main() {

    struct dirent *dir; 
    struct stat statbuf;
    char datestring[256];
    char path[500];
    struct tm *dt;
    time_t t2;
    double time_dif;
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
            sprintf(filename, "%s",dir->d_name);
            sprintf(f_path, "%s/%s", path, dir->d_name);
            printf("filename: %s", filename);
            printf("  filepath: %s\n", f_path);

            if(stat(f_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }

            printf("file_name: ");
            for(int i=0; i < sizeof(filename); i++) {
                
                printf("%c ", filename[i]);
                if(filename[i] == '\0') {
                    break;
                }
            }
            printf("\n");
            printf("dir_path: ");
            for(int i=0; i < sizeof(f_path); i++) {
                
                printf("%c ", f_path[i]);
                if(f_path[i] == '\0') {
                    break;
                }
            }
            printf("\n");
            
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            char d_path[500]; 
            char dir_name[256];
            sprintf(d_path, "%s/%s", path, dir->d_name);
            sprintf(dir_name, "%s", dir->d_name);
            printf("dir_name: ");
            for(int i=0; i < sizeof(dir_name); i++) {
                
                printf("%c ", dir_name[i]);
                if(dir_name[i] == '\0') {
                    break;
                }
            }
            printf("\n");
            printf("dir_path: ");
            for(int i=0; i < sizeof(d_path); i++) {
                
                printf("%c ", d_path[i]);
                if(d_path[i] == '\0') {
                    break;
                }
            }
            printf("\n");

            show_dir_content(d_path); 
        }
    }
    closedir(d); 

    
}