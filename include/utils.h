#ifndef _UTIL_H
#define _UTIL_H
#include <stdio.h>
/**********************************************
 * init
   - port is the number of the port you want the server to be
     started on
   - initializes the connection acception/handling system
   - YOU MUST CALL THIS EXACTLY ONCE (not once per thread,
     but exactly one time, in the main thread of your program)
     BEFORE USING ANY OF THE FUNCTIONS BELOW
   - if init encounters any errors, it will call exit().
************************************************/
void init(int port);

/**********************************************
 * accept_connection - takes no parameters
   - returns a file descriptor for further request processing.
     DO NOT use the file descriptor on your own -- use
     get_request() instead.
   - if the return value is negative, the request should be ignored.
***********************************************/
int accept_connection(void);

/**********************************************
 * get_request
    - parameters:
        - fd is the file descriptor of the connection
        - filelength is a pointer to a size_t
    - returns:
        - a string containing the request from the client
        - the length of the file is stored in *filelength
    - the string must be freed by the caller
    - if the return value is NULL, the request should be ignored.
************************************************/
char * get_request_server(int fd, size_t *filelength);

/*
 * setup_connection
 * - parameters:
 *     - port is the port number to connect to
 * - returns:
 *    - a file descriptor for the connection
*/
int setup_connection(int port);

/*
  * send_file_to_client
  * - parameters:
  *   - socket is the file descriptor of the connection
  *   - buffer is the buffer containing the file
  *   - size is the size of the file
  * - returns:
  *   - 0 on success, -1 on failure
*/
int send_file_to_client(int socket, char * buffer, int size);


/*
  * send_file_to_server
  * - parameters:
  *  - socket is the file descriptor of the connection
  *  - file is the file to send
  *  - size is the size of the file
*/
int send_file_to_server(int socket, FILE *file, int size);


/*
  * receive_file_from_client
  * - parameters:
  *    - socket is the file descriptor of the connection
  *   - filename is the name of the file to save
  * - returns:
  *   - 0 on success, -1 on failure
*/
int receive_file_from_server(int socket, const char *filename);
#endif /* _UTIL_H */