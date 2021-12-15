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

    struct dirent *dp;
    struct stat statbuf;
    char datestring[256];
    struct tm *tm;
    time_t t1 = 0, t2;
    double time_dif;


    DIR *dir;
    char *filename;
    //char *path = "/opt/Testing/Data";
    dir = opendir("."); //Enter directory path
    //Loop through directory entries
    while((dp = readdir(dir)) != NULL) {

        filename = dp->d_name;
        printf("filename: %s  ", filename);
        if(stat(dp->d_name, &statbuf) == -1) {
            fprintf(stderr,"Error: %s\n", strerror(errno));
            continue;
        }

        tm = gmtime(&statbuf.st_mtime); 
        t2 = statbuf.st_mtime;
        strftime(datestring, sizeof(datestring), " %x-%X", tm);
        //printf("datestring: %s\n", datestring);
        
        time_dif = difftime(t2, t1);
        if(time_dif > 0) {
            printf("datestring: %s\n", datestring);
        }

        t1 = t2;

    }
}