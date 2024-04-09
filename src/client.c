#include "../include/client.h"



int port = 0;

pthread_t worker_thread[100];
int worker_thread_id = 0;
char output_path[1028];

processing_args_t req_entries[100];

char* get_file_name(char *path) {
    char *file_name = strrchr(path, '/');
    if (file_name == NULL) {
        return path;
    }
    return file_name + 1;
}

/* TODO: implement the request_handle function to send the image to the server and recieve the processed image
* 1. Open the file in the read-binary mode - Intermediate Submission
* 2. Get the file length using the fseek and ftell functions - Intermediate Submission
* 3. set up the connection with the server using the setup_connection(int port) function - Intermediate Submission
* 4. Send the file to the server using the send_file_to_server(int socket, FILE *fd, size_t size) function - Intermediate Submission
* 5. Receive the processed image from the server using the receive_file_from_server(int socket, char *file_path) function
* 6. receive_file_from_server saves the processed image in the output directory, so pass in the right directory path
* 7. Close the file and the socket
*/
void * request_handle(void * args) {
    processing_args_t *processing_args = (processing_args_t *) args;
    char *file_name = processing_args->file_name;
    FILE *file = fopen(file_name, "rb");
    if (!file) {
        fprintf(stderr, "%s at %d: Failed to open file %s\n", __FILE__, __LINE__, file_name);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "%s at %d: Failed to seek to the end of the file %s\n", __FILE__, __LINE__, file_name);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    size_t file_size = ftell(file);
    if (file_size == -1) {
        fprintf(stderr, "%s at %d: Failed to get the file size of %s\n", __FILE__, __LINE__, file_name);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    rewind(file);

    int socket = setup_connection(port);
    send_file_to_server(socket, file, file_size);

    char *out_path = (char *) malloc(sizeof(char) * 1028);
    sprintf(out_path, "%s/%s", output_path, get_file_name(file_name));
    receive_file_from_server(socket, out_path);

    free(out_path);
    // close(socket);
    fclose(file);
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
void directory_trav(char * args) {
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;

    if(!(dir = opendir(args))) {
        fprintf(stderr, "%s at %d: Failed to open directory %s\n", __FILE__, __LINE__, args);
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char path[BUFF_SIZE];
            snprintf(path, sizeof(path), "%s/%s", args, entry->d_name);
            if (stat(path, &filestat) < 0) {
                fprintf(stderr, "%s at %d: Failed to get file stats for %s\n", __FILE__, __LINE__, path);
                exit(EXIT_FAILURE);
            }
            if (S_ISREG(filestat.st_mode)) {    // if regular file
                // create a new thread to invoke the request_handle function
                // pass the file path as an argument
                strcpy(req_entries[worker_thread_id].file_name, path);
                req_entries[worker_thread_id].number_worker = worker_thread_id;
                pthread_create(&worker_thread[worker_thread_id], NULL, request_handle, (void *) &req_entries[worker_thread_id]);
                worker_thread_id++;
            }
        } else {
            continue;
        }
    }

    for (int i = 0; i < worker_thread_id; i++) {
        pthread_join(worker_thread[i], NULL);
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: ./client <directory path> <Server Port> <output path>\n");
        exit(-1);
    }
    /*TODO:  Intermediate Submission
     * 1. Get the input args --> (1) directory path (2) Server Port (3) output path
     */
    port = atoi(argv[2]);
    strcpy(output_path, argv[3]);
    /*TODO: Intermediate Submission
     * Call the directory_trav function to traverse the directory and send the images to the server
     */
    for (int i = 0; i < 100; i++) {
        req_entries[i].file_name = (char *) malloc(sizeof(char) * 1028);
    }
    directory_trav(argv[1]);
    // printf("All images processed successfully\n");
    int fd = setup_connection(port);
    if (fd == -1) {
        fprintf(stderr, "%s at %d: Failed to connect to the server\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 100; i++) {
        free(req_entries[i].file_name);
    }
    close(fd);
    return 0;  
}