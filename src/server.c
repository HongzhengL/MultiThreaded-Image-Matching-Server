#include "../include/server.h"

#define MAX_DATABASE_SIZE 100

// /********************* [ Helpful Global Variables ] **********************/
int num_dispatcher = 0;  // Global integer to indicate the number of dispatcher threads
int num_worker = 0;  // Global integer to indicate the number of worker threads
FILE *logfile;  // Global file pointer to the log file
int queue_len = 0;  // Global integer to indicate the length of the queue


database_entry_t database[MAX_DATABASE_SIZE];
int database_size = 0;

pthread_t *dispatcher_threads;
pthread_t *worker_threads;

pthread_mutex_t req_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;
typedef struct {
    request_t *requests;
    int head;
    int tail;
    int size;
    int capacity;
} simple_queue_t;

simple_queue_t req_queue;

/**
 * @brief Initialize the queue with given capacity
 * 
 * @param queue pointer to a queue to be initialized
 * @param capacity capacity of the queue
 */
void queue_init(simple_queue_t *queue, int capacity) {
    queue->requests = (request_t *) malloc(capacity * sizeof(request_t));
    if (!queue->requests) {
        fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < capacity; i++) {
        queue->requests[i].buffer = (char *)malloc(BUFFER_SIZE);
        if (!queue->requests[i].buffer) {
            fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
        }
    }
    queue->head = 0;
    queue->tail = 0;
    queue->size = 0;
    queue->capacity = capacity;
}

/**
 * @brief free the memory allocated for the queue
 * 
 * @param queue pointer to a queue to be destroyed
 */
void queue_destroy(simple_queue_t *queue) {
    for (int i = 0; i < queue->capacity; i++)
        if (queue->requests[i].buffer)
            free(queue->requests[i].buffer);
    if (queue->requests)
        free(queue->requests);
}

/**
 * @brief put a request into the queue, update it using circular fashion
 *        use mutex to protect the queue from concurrent access
 *        and use condition variable to signal that the queue is not empty
 * 
 * @param queue pointer to queue to put the request into
 * @param request request to be put into the queue
 */
void enqueue(simple_queue_t *queue, request_t request) {
    int error = 0;  // store the error code from pthread functions

    // Request thread safe access to the request queue
    if ((error = pthread_mutex_lock(&req_queue_mutex))) {
        fprintf(stderr, "%s at %d: pthread_mutex_lock() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
    while (queue->size == queue->capacity) {
        if ((error = pthread_cond_wait(&not_full, &req_queue_mutex))) {
            fprintf(stderr, "%s at %d: pthread_cond_wait() return %d\n", __FILE__, __LINE__, error);
            exit(EXIT_FAILURE);
        }
    }

    // Copy the request into the queue
    queue->requests[queue->tail].file_size = request.file_size;
    queue->requests[queue->tail].file_descriptor = request.file_descriptor;
    memcpy(queue->requests[queue->tail].buffer, request.buffer, request.file_size);
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->size++;

    // Signal that the queue is not empty and release the lock
    if ((error = pthread_cond_signal(&not_empty))) {
        fprintf(stderr, "%s at %d: pthread_cond_signal() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
    if ((error = pthread_mutex_unlock(&req_queue_mutex))) {
        fprintf(stderr, "%s at %d: pthread_mutex_unlock() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief remove a request from the queue, update it using circular fashion
 *        use mutex to protect the queue from concurrent access
 *        and use condition variable to signal that the queue is not full
 * 
 * @param queue pointer to queue to remove the request from
 * @return request_t return the request removed from the queue
 */
request_t deque(simple_queue_t *queue) {
    int error = 0;  // store the error code from pthread functions

    // Request thread safe access to the request queue
    if ((error = pthread_mutex_lock(&req_queue_mutex))) {
        fprintf(stderr, "%s at %d: pthread_mutex_lock() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
    while (queue->size == 0) {
        if ((error = pthread_cond_wait(&not_empty, &req_queue_mutex))) {
            fprintf(stderr, "%s at %d: pthread_cond_wait() return %d\n", __FILE__, __LINE__, error);
            exit(EXIT_FAILURE);
        }
    }

    // Copy the request from the queue
    request_t request;
    request.buffer = (char *)malloc(queue->requests[queue->head].file_size);
    if (!request.buffer) {
        fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    request.file_size = queue->requests[queue->head].file_size;
    request.file_descriptor = queue->requests[queue->head].file_descriptor;
    memcpy(request.buffer, queue->requests[queue->head].buffer, queue->requests[queue->head].file_size);

    // Update the queue remove index in a circular fashion
    queue->head = (queue->head + 1) % queue->capacity;
    queue->size--;

    // Signal that the queue is not full and release the lock
    if ((error = pthread_cond_signal(&not_full))) {
        fprintf(stderr, "%s at %d: pthread_cond_signal() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
    if ((error = pthread_mutex_unlock(&req_queue_mutex))) {
        fprintf(stderr, "%s at %d: pthread_mutex_unlock() return %d\n", __FILE__, __LINE__, error);
        exit(EXIT_FAILURE);
    }
    return request;
}

/**
 * @brief compare the input_image with the images in the database
 * 
 * @param input_image a pointer to the input image
 * @param size size of the input_image
 * @return database_entry_t an entry in the database 
 *         that is the closest match to the input_image
 */

database_entry_t image_match(char *input_image, int size) {
    const char *closest_file     = NULL;
    int         closest_distance = INT_MAX;
    int closest_index = 0;
    int closest_file_size = INT_MAX;
    for (int i = 0; i < database_size; i++) {
        const char *current_file;
        current_file = database[i].buffer;
        int result = memcmp(input_image, current_file, size);
        if (result == 0) {
            return database[i];
        } else if (result < closest_distance || (result == closest_distance && database[i].file_size < closest_file_size)) {
            closest_distance = result;
            closest_file     = current_file;
            closest_index = i;
            closest_file_size = database[i].file_size;
        }
    }

    if (closest_file != NULL) {
      return database[closest_index];
    } else {
      return database[closest_index];
    }
}

/**
 * @brief print the log message to the file pointer to_write and to the terminal
 * 
 * @param to_write file pointer to write the log message
 * @param threadId id of the thread that is logging the message
 * @param requestNumber number of the request in the thread
 * @param fd file descriptor of the thread sending backing information
 * @param file_name file name of the file being requested in the thread
 * @param file_size file size of the file being requested in the thread
 */
void LogPrettyPrint(FILE* to_write, int threadId, int requestNumber, int fd, char * file_name, int file_size) {
    if (to_write == NULL) {
        fprintf(stderr, "[%d] [%d] [%d] [%s] [%d]\n", threadId, requestNumber, fd, file_name, file_size);
    } else {
        fprintf(to_write, "[%d] [%d] [%d] [%s] [%d]\n", threadId, requestNumber, fd, file_name, file_size);
    }
}

/**
 * @brief Traverse the directory and load all the images into the database
 *        store the file name, file size and the image data in the database
 *        increment the global variable storing the number of images in the database
 * 
 * @param path path to the directory containing the images
 */
void loadDatabase(char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;

    if (!(dir = opendir(path))) {
        fprintf(stderr, "%s at %d: Could not open directory %s\n", __FILE__, __LINE__, path);
        exit(EXIT_FAILURE);
    }

    while ((entry = readdir(dir)) != NULL) {
        char full_path[BUFF_SIZE];

        if (entry->d_type == DT_REG) {  // If the entry is a regular file
            snprintf(full_path, BUFF_SIZE, "%s/%s", path, entry->d_name);

            FILE *file = fopen(full_path, "rb");
            if (!file) {
                fprintf(stderr, "%s at %d: Could not open file %s\n", __FILE__, __LINE__, full_path);
                exit(EXIT_FAILURE);
            }

            if (fstat(fileno(file), &fileStat) < 0) {
                fprintf(stderr, "%s at %d: Could not get file stats for %s\n", __FILE__, __LINE__, full_path);
                exit(EXIT_FAILURE);
            }

            // Read the file into a buffer
            char *buffer = (char *)malloc(fileStat.st_size);
            if (!buffer) {
                fprintf(stderr, "%s at %d: Could not allocate memory for file %s\n", __FILE__, __LINE__, full_path);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            if (fread(buffer, 1, fileStat.st_size, file) != fileStat.st_size) {
                fprintf(stderr, "%s at %d: Could not read file %s\n", __FILE__, __LINE__, full_path);
                free(buffer);
                fclose(file);
                exit(EXIT_FAILURE);
            }

            // Store the file name, file size and the image data in the database
            database[database_size].file_name = (char *)malloc(strlen(entry->d_name) + 1);
            if (!database[database_size].file_name) {   // If malloc fails
                fprintf(stderr, "%s at %d: Could not allocate memory for file name\n", __FILE__, __LINE__);
                free(buffer);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            strcpy(database[database_size].file_name, entry->d_name);
            database[database_size].file_size = fileStat.st_size;
            database[database_size].buffer = (char *)malloc(fileStat.st_size);
            if (!database[database_size].buffer) {  // If malloc fails
                fprintf(stderr, "%s at %d: Could not allocate memory for file buffer\n", __FILE__, __LINE__);
                free(database[database_size].file_name);
                free(buffer);
                fclose(file);
                exit(EXIT_FAILURE);
            }
            memcpy(database[database_size].buffer, buffer, fileStat.st_size);

            database_size++;

            free(buffer);
            fclose(file);
        }
    }
    closedir(dir);
}

/**
 * @brief Accept client connection, put the request into the queue
 * 
 * @param arg pointer to the thread id
 * @return void* 
 */
void *dispatch(void *arg)  {
    while (1) {
        size_t file_size = 0;
        request_details_t request_detail;

        // Accept client connection and check if the connection is successful
        int socketfd = accept_connection();
        if (socketfd < 0) {
            continue;
        }

        // Get the request buffer and file size from the client
        char *request = get_request_server(socketfd, &file_size);
        if (request == NULL) {  // if request is not valid
            continue;
        } else {
            request_detail.filelength = file_size;
            memcpy(request_detail.buffer, request, file_size);
        }

        // Put the request into the queue
        request_t queue_request;
        queue_request.file_size = file_size;
        queue_request.file_descriptor = socketfd;
        queue_request.buffer = (char *)malloc(file_size);
        if (!queue_request.buffer) {
            fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
            free(request);
            close(socketfd);
            exit(EXIT_FAILURE);
        }
        memcpy(queue_request.buffer, request_detail.buffer, file_size);

        // enqueue is a thread safe function
        enqueue(&req_queue, queue_request);
        free(queue_request.buffer);
        free(request);
    }
    return NULL;
}

/**
 * @brief dequeue the request from the queue
 *        process the request and send the file to the client
 * 
 * @param arg pointer to the thread id
 * @return void* 
 */
void *worker(void *arg) {
    int num_request = 0;                                    // Integer for tracking each request for printing into the log file
    int fileSize    = 0;                                    // Integer to hold the size of the file being requested
    void *memory    = NULL;                                 // memory pointer where contents being requested are read and stored
    int fd          = INVALID;                              // Integer to hold the file descriptor of incoming request
    char *mybuf;                                  // String to hold the contents of the file being requested


    // Get the thread id
    int ID = *((int *) arg);

    while (1) {
        // Dequeue the request from the queue
        request_t request = deque(&req_queue);
        fd = request.file_descriptor;
        fileSize = request.file_size;

        if (fileSize == 0) {    // if the file is not a valid file
            continue;
        }

        // read the request buffer into a memory buffer
        mybuf = (char *)malloc(fileSize);
        if (mybuf == NULL) {
            fprintf(stderr, "%s at %d: Could not allocate memory for file\n", __FILE__, __LINE__);
            close(fd);
            continue;
        }
        memcpy(mybuf, request.buffer, fileSize);
        memory = (char *)malloc(fileSize);
        if (memory == NULL) {
            fprintf(stderr, "%s at %d: Could not allocate memory for file\n", __FILE__, __LINE__);
            close(fd);
            continue;
        }
        memcpy(memory, mybuf, fileSize);

        // Process the request and send the file to the client
        database_entry_t result = image_match(mybuf, fileSize);
        send_file_to_client(fd, result.buffer, result.file_size);

        // Clean up and log the request
        free(mybuf);
        free(memory);
        num_request++;
        LogPrettyPrint(logfile, ID, num_request, fd, result.file_name, result.file_size);
        LogPrettyPrint(NULL, ID, num_request, fd, result.file_name, result.file_size);
        free(request.buffer);
    }
    return NULL;
}

/**
 * @brief Clean up the resources and exit the program
 * 
 * @param signum signal number
 */
void signal_handler(int signum) {
    if (logfile) {
        fflush(logfile);
        fclose(logfile);
    }
    if (dispatcher_threads)
        free(dispatcher_threads);
    if (worker_threads)
        free(worker_threads);
    queue_destroy(&req_queue);
    for (int i = 0; i < database_size; ++i) {
        if (database[i].file_name)
            free(database[i].file_name);
        if (database[i].buffer)
            free(database[i].buffer);
    }
    exit(EXIT_SUCCESS);
}

int main(int argc , char *argv[]) {
    if (argc != 6) {
        printf("usage: %s port path num_dispatcher num_workers queue_length \n", argv[0]);
        return -1;
    }

    // Register the signal handler
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "%s at %d: sigaction() return with %d\n", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &action, NULL)) {
        fprintf(stderr, "%s at %d: sigaction() return with %d\n", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &action, NULL)) {
        fprintf(stderr, "%s at %d: sigaction() return with %d\n", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &action, NULL)) {
        fprintf(stderr, "%s at %d: sigaction() return with %d\n", __FILE__, __LINE__, errno);
        exit(EXIT_FAILURE);
    }

    int port            = -1;
    char path[BUFF_SIZE] = "no path set\0";
    num_dispatcher      = -1;                               // global variable
    num_worker          = -1;                               // global variable
    queue_len           = -1;                               // global variable


    // Read the input arguments
    port = atoi(argv[1]);
    if (port < MIN_PORT || port > MAX_PORT) {
        fprintf(stderr, "%s: Invalid port number\n", __FILE__);
        exit(EXIT_FAILURE);
    }
    strcpy(path, argv[2]);
    num_dispatcher = atoi(argv[3]) < MAX_THREADS ? atoi(argv[3]) : MAX_THREADS;
    num_worker = atoi(argv[4]) < MAX_THREADS ? atoi(argv[4]) : MAX_THREADS;
    queue_len = atoi(argv[5]) < MAX_QUEUE_LEN ? atoi(argv[5]) : MAX_QUEUE_LEN;
    if (num_dispatcher < 1 || num_worker < 1 || queue_len < 1) {
        fprintf(stderr, "%s: Invalid arguments\n", __FILE__);
        exit(EXIT_FAILURE);
    }

    // initialize the request queue
    queue_init(&req_queue, queue_len);

    // Open the log file
    logfile = fopen(LOG_FILE_NAME, "w");
    if (logfile == NULL) {
        fprintf(stderr, "%s at %d: fopen could not open log file\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }

    // Initialize the server
    init(port);

    // Initialize and load the database
    for (int i = 0; i < MAX_DATABASE_SIZE; ++i) {
        database[i].file_name = NULL;
        database[i].file_size = 0;
        database[i].buffer = NULL;
    }
    loadDatabase(path);

    // Create dispatcher and worker threads
    dispatcher_threads = (pthread_t *) malloc(num_dispatcher * sizeof(pthread_t));
    if (!dispatcher_threads) {
        fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    int dispatcher_thread_ids[num_dispatcher];
    for (int i = 0; i < num_dispatcher; ++i) {
        dispatcher_thread_ids[i] = i;
        int rc = pthread_create(&dispatcher_threads[i], NULL, dispatch, (void *) &dispatcher_thread_ids[i]);
        if (rc) {
            fprintf(stderr, "%s at %d: pthread_create() return %d\n", __FILE__, __LINE__, rc);
            free(dispatcher_threads);
            exit(EXIT_FAILURE);
        }
    }

    worker_threads = (pthread_t *) malloc(num_worker * sizeof(pthread_t));
    if (!worker_threads) {
        fprintf(stderr, "%s at %d: malloc() return NULL\n", __FILE__, __LINE__);
        exit(EXIT_FAILURE);
    }
    int thread_nums[num_worker];
    for (int j = 0; j < num_worker; ++j) {
        thread_nums[j] = j;
        int rc = pthread_create(&worker_threads[j], NULL, worker, (void *) &thread_nums[j]);
        if (rc) {
            fprintf(stderr, "%s at %d: pthread_create() return %d\n", __FILE__, __LINE__, rc);
            free(dispatcher_threads);
            free(worker_threads);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for each of the threads to complete their work
    // Threads (if created) will not exit (see while loop), but this keeps main from exiting
    int i;
    for (i = 0; i < num_dispatcher; i++) {
        fprintf(stderr, "JOINING DISPATCHER %d \n", i);
        if ((pthread_join(dispatcher_threads[i], NULL)) != 0) {
            printf("ERROR : Fail to join dispatcher thread %d.\n", i);
        }
    }
    for (i = 0; i < num_worker; i++) {
    // fprintf(stderr, "JOINING WORKER %d \n",i);
        if ((pthread_join(worker_threads[i], NULL)) != 0) {
            printf("ERROR : Fail to join worker thread %d.\n", i);
        }
    }

    // Clean up the resources when accidentally exit
    fprintf(stderr, "SERVER DONE \n");  // will never be reached in SOLUTION
    fclose(logfile);  // closing the log files
    queue_destroy(&req_queue);
    free(dispatcher_threads);
    free(worker_threads);
}
