#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>


/* let us make a recursive function to print the content of a given folder */

void show_dir_content(char * path)
{
    struct dirent * dir; // for the directory entries
    struct stat statbuf;
    char datestring[256];
    char *filename;
    struct tm *tm;
    DIR * d = opendir(path); // open the path
    if(d==NULL) {
        return;  
    }
    // if we were able to read somehting from the directory
    while ((dir = readdir(d)) != NULL) 
    {
        // if the type is not directory
        if(dir-> d_type != DT_DIR) {

            filename = dir->d_name;
            printf("filename: %s", filename);
            printf("  filepath: %s\n", path);

            if(stat(dir->d_name, &statbuf) == -1) {
                continue;
            }
            tm = gmtime(&statbuf.st_mtime);
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("%s\n", dir->d_name);
            char d_path[500]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
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