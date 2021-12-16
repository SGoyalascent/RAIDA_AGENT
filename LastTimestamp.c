//Compare the two timestamps and get the newest timestamp


#include <stdio.h> 
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>

struct timestamp {

    unsigned char year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minutes;
    unsigned char second;
};

struct timestamp tm;


/* let us make a recursive function to print the content of a given folder */

void show_dir_content(char * path)
{
    struct dirent *dir; 
    struct stat statbuf;
    char datestring[256];
    struct tm *dt;
    time_t t1 = 0, t2;
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
            //printf("  filepath: %s\n", f_path);

            if(stat(f_path, &statbuf) == -1) {
                fprintf(stderr,"Error: %s\n", strerror(errno));
                continue;
            }
            dt = gmtime(&statbuf.st_mtime);
            t2 = statbuf.st_mtime;
            strftime(datestring, sizeof(datestring), " %x-%X", dt);
            printf("datestring: %s\n", datestring);

            time_dif = difftime(t2, t1);
            printf("time_diff: %g\n", time_dif);
            if(time_dif > 0) {
                t1 = t2;
                printf("datestring: %s  ", datestring);

                tm.year = dt->tm_year - 100;
                tm.month = dt->tm_mon;
                tm.day = dt->tm_mday;
                tm.hour = dt->tm_hour;
                tm.minutes = dt->tm_min;
                tm.second = dt->tm_sec;
                printf("Last Modified Time(UTC):  %d-%d-%d  %d:%d:%d\n",tm.day, tm.month,tm.year, tm.hour, tm.minutes, tm.second);
                //printf("Last Modified Time2:- %d-%d-%d  %d:%d:%d\n",dt->tm_mday,dt->tm_mon,dt->tm_year+1900, dt->tm_hour,dt->tm_min, dt->tm_sec);
            }
        }

        // if it is a directory
        if(dir -> d_type == DT_DIR && strcmp(dir->d_name,".")!=0 && strcmp(dir->d_name,"..")!=0 ) 
        {
            printf("directory: %s ", dir->d_name);
            char d_path[500]; 
            sprintf(d_path, "%s/%s", path, dir->d_name);
            printf("  dirpath: %s\n", d_path);
            show_dir_content(d_path); // recall with the new path
        }
    }
    closedir(d);
}

int main()
{
    char *path = "/opt/Testing/Data";
    show_dir_content(path);
    return(0);
}