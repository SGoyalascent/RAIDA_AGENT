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
#include <errno.h>
#include <stdint.h>


int main() {

	char execpath[500] =  "/opt/Testing/";
    char path[50];
	char paths[50];
    
    strcpy(path, execpath);
	strcat(paths, execpath);
    printf("path: %s  paths: %s\n", path, paths);
}