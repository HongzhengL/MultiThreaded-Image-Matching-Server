#include "../include/client.h"



int port = 0;

pthread_t worker_thread[100];
int worker_thread_id = 0;
char output_path[1028];

processing_args_t req_entries[100];

/* TODO: implement the request_handle function to send the image to the server and recieve the processed image
* 1. Open the file in the read-binary mode - Intermediate Submission
* 2. Get the file length using the fseek and ftell functions - Intermediate Submission
* 3. set up the connection with the server using the setup_connection(int port) function - Intermediate Submission
* 4. Send the file to the server using the send_file_to_server(int socket, FILE *fd, size_t size) function - Intermediate Submission
* 5. Receive the processed image from the server using the receive_file_from_server(int socket, char *file_path) function
* 6. receive_file_from_server saves the processed image in the output directory, so pass in the right directory path
* 7. Close the file and the socket
*/
void * request_handle(void * args)
{
    return NULL;
}

/* TODO: Intermediate Submission
* implement the directory_trav function to traverse the directory and send the images to the server 
* 1. Open the directory
* 2. Read the directory entries
* 3. If the entry is a file, create a new thread to invoke the request_handle function which takes the file path as an argument
* 4. Join all the threads
* Note: Make sure to avoid any race conditions when creating the threads and passing the file path to the request_handle function. 
* use the req_entries array to store the file path and pass the index of the array to the thread. 
*/
void directory_trav(char * args)
{
   
}
int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: ./client <directory path> <Server Port> <output path>\n");
        exit(-1);
    }
    /*TODO:  Intermediate Submission
    * 1. Get the input args --> (1) directory path (2) Server Port (3) output path
    */

    /*TODO: Intermediate Submission
    * Call the directory_trav function to traverse the directory and send the images to the server
    */
    return 0;  
}