#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <stdint.h>
#include "utils.h"


#define BUFF_SIZE 1028
#define BUFFER_SIZE 1028
#define MAX 80
#define SA struct sockaddr

typedef struct processing_args
{
    int number_worker;
    char *file_name;
} processing_args_t;


typedef struct packet {
    unsigned int size;
} packet_t; 

typedef struct request_detials
{
    char buffer[1028];
    long filelength;
    char file_name[1028];
} request_details_t;


/********************* [ Function Prototypes       ] **********************/
void * request_handle(void * args);
void directory_trav(char * args);