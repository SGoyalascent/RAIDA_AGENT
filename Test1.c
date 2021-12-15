// Mirror_Report_Changes---------Check Changes in the Directory

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


int main() {

    struct dirent *dp;
    struct stat statbuf;
    char datestring[256];
    struct tm *tm;

    DIR *dir;
    char *filename;
    char *path = "/opt/Testing/Data";
    dir = opendir("."); //Enter directory path
    //Loop through directory entries
    while((dp = readdir(dir)) != NULL) {

        filename = dp->d_name;
        printf("filename: %s\n", filename);
        //Get Entry's information
        if(stat(dp->d_name, &statbuf) == -1) {
            fprintf(stderr,"Error: %d  %s\n", errno, strerror(errno));
            continue;
        }

        tm = gmtime(&statbuf.st_mtime);
        

        /*
        strftime(datestring, sizeof(datestring), " %x-%X", tm);
        printf("datestring1: %s of filename: %s\n", datestring, dp->d_name);

        printf("datestring1: ");
        for(int i=0; i < sizeof(datestring); i++) {
             printf("%d  ", datestring[i]);
        }
        printf("\n");

        strftime(datestring, sizeof(datestring), " %d-%m-%y-%Y  %H:%M:%S", tm);
        printf("datestring2: %s of filename: %s\n", datestring, dp->d_name);

        printf("datestring2: ");
        for(int i=0; i < sizeof(datestring); i++) {
             printf("%d  ", datestring[i]);
        }
        printf("\n");
        */
    }




}