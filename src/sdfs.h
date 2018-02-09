#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <time.h>
#include <unistd.h>

#define LINE 			4096
#define LISTENQ 		1023
#define SA 				struct sockaddr
#define SELECT_TIMEOUT 	2
#define ANSI_RESET 		"\e[0m"
#define ANSI_WORNING 	"\e[41;97m"
#define ANSI_HEADING  "\e[33;40m"
