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
    char *filepath;
    struct tm *tm;
    DIR * d = opendir(path); // open the path
    printf(" path: %s\n", path);
    if(d==NULL) {
        return;  
    }
    // if we were able to read somehting from the directory
    while ((dir = readdir(d)) != NULL) 
    {
        // if the type is not directory
        if(dir-> d_type != DT_DIR) {
            
            filename = dir->d_name;
            sprintf(filepath, "%s/%s", path, dir->d_name);
            printf("filename: %s", filename);
            printf("  filepath: %s\n", filepath);

            if(stat(dir->d_name, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            tm = gmtime(&statbuf.st_mtime);
            strftime(datestring, sizeof(datestring), " %x-%X", tm);
            printf("datestring1: %s of filename: %s\n", datestring, dir->d_name);
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("%s ", dir->d_name);
            char d_path[500]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
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