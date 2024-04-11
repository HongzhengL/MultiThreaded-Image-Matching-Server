#include "../include/client.h"



int port = 0;

pthread_t worker_thread[100];
int worker_thread_id = 0;
char output_path[1028];

processing_args_t req_entries[100];

/**
 * @brief Get the file name object
 * 
 * @param path a string containing the path to the file
 * @return char* 
 */
char* get_file_name(char *path) {
    char *file_name = strrchr(path, '/');
    if (file_name == NULL) {
        return path;
    }
    return file_name + 1;
}

/**
 * @brief create request to send the image to the server and receive the processed image
 * 
 * @param args pointer to the processing_args_t struct containing all the necessary information
 * @return void* 
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
    if (out_path == NULL) {
        fprintf(stderr, "%s at %d: Failed to allocate memory for out_path\n", __FILE__, __LINE__);
        fclose(file);
        exit(EXIT_FAILURE);
    }
    sprintf(out_path, "%s/%s", output_path, get_file_name(file_name));
    receive_file_from_server(socket, out_path);

    free(out_path);
    fclose(file);
    if (close(socket) < 0) {
        fprintf(stderr, "%s at %d: close() failed, errno = %d\n", __FILE__, __LINE__, errno);
    }
    return NULL;
}

/**
 * @brief traverse the directory and send each image to the server to be processed
 * 
 * @param args the path to the directory
 */
void directory_trav(char * args) {
    DIR *dir;
    struct dirent *entry;
    struct stat filestat;
    int error = 0;      // get the error code from pthread functions

    if (!(dir = opendir(args))) {
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
                if ((error = pthread_create(&worker_thread[worker_thread_id], NULL, request_handle, (void *) &req_entries[worker_thread_id]))) {
                    fprintf(stderr, "%s at %d: Failed to create thread for %s\n", __FILE__, __LINE__, path);
                    exit(EXIT_FAILURE);
                }
                worker_thread_id++;
            }
        } else {
            continue;
        }
    }

    for (int i = 0; i < worker_thread_id; i++) {
        if ((error = pthread_join(worker_thread[i], NULL))) {
            fprintf(stderr, "%s at %d: Failed to join thread %d\n", __FILE__, __LINE__, i);
            exit(EXIT_FAILURE);
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: ./client <directory path> <Server Port> <output path>\n");
        exit(-1);
    }
    // parse the command line arguments
    port = atoi(argv[2]);
    strcpy(output_path, argv[3]);

    // initialize the req_entries array, and traverse the directory to send the images to the server
    for (int i = 0; i < sizeof(req_entries) / sizeof(req_entries[0]); i++) {
        req_entries[i].file_name = (char *) malloc(sizeof(char) * BUFF_SIZE);
        if (req_entries[i].file_name == NULL) {
            fprintf(stderr, "%s at %d: Failed to allocate memory for req_entries[%d].file_name\n", __FILE__, __LINE__, i);
            exit(EXIT_FAILURE);
        }
    }
    directory_trav(argv[1]);

    // setup connection to the server and send the images to be processed
    int fd = setup_connection(port);
    if (fd == -1) {
        fprintf(stderr, "%s at %d: Failed to connect to the server\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // clean up resources
    for (int i = 0; i < 100; i++) {
        if (req_entries[i].file_name) {
            free(req_entries[i].file_name);
        }
    }
    if (close(fd) == -1) {
        fprintf(stderr, "%s at %d: Failed to close the socket\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    return 0;
}
