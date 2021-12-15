#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>


/* let us make a recursive function to print the content of a given folder */

void show_dir_content(char * path)
{
    struct dirent * dir; // for the directory entries
    struct stat statbuf;
    char datestring[256];
    char *filename;
    struct tm *tm;
    DIR * d = opendir(path); // open the path
    printf(" path: %s\n", path);
    if(d==NULL) {
        return;  
    }
    // if we were able to read somehting from the directory
    while ((dir = readdir(d)) != NULL) 
    {
        printf("ck1 ");
        // if the type is not directory
        if(dir-> d_type != DT_DIR) {

            printf("ck2 ");
            char *filepath;
            printf("ck3 ");
            filename = dir->d_name;
            printf("ck4 ");
            sprintf(filepath, "%s/%s", path, dir->d_name);
            printf("ck5\n");
            printf("filename: %s", filename);
            printf("ck6\n");
            printf("  filepath: %s\n", filepath);
            printf("ck7\n");

            if(stat(filepath, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            printf("ck8");
            tm = gmtime(&statbuf.st_mtime);
            printf("ck9");
            strftime(datestring, sizeof(datestring), " %x-%X", tm);
            printf("ck10\n");
            printf("datestring1: %s of filename: %s\n", datestring, dir->d_name);
            printf("ck11\n");
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("ck12\n");
            printf("%s ", dir->d_name);
            printf("ck13  ");
            char d_path[500]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
            printf("ck14\n");
            printf("  dirpath: %s\n", d_path);
            show_dir_content(d_path); // recall with the new path
        }
    }
    closedir(d); // finally close the directory
}

int main()
{
    char *path = "/opt/Testing/Data";
    show_dir_content(path);
    return(0);
}