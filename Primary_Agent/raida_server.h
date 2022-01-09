#ifndef AGENT_SERVER_H
#define AGENT_SERVER_H
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>

//--------------------------------------------
#define RAIDA_SERVER_MAX 					25
//--------------------------------------------
#define SERVER_CONFIG_BYTES 				4
#include "udp_socket.h"
#include "aes.h"

struct server_config {
	unsigned char raida_id;
	unsigned int port_number;
	unsigned int bytes_per_frame;
};

union bytes {
    unsigned int val;
    unsigned char byte[2];
};


void welcomeMsg();
int load_raida_no();
int load_server_config();
long get_time_cs();
extern char execpath[256];
extern struct server_config server_config_obj;
extern union bytes binary;
#endif
