# CSCI 4061 Project 3

## MultiThreaded Image Matching Server




### Project group number

- Group 44

### Group member names and x500s

- Elias Vera-Jimenez (veraj002)
- Hongzheng Li (li003458)

### The name of the CSELabs computer that you tested your code on

- csel-kh1250-13.cselabs.umn.edu

### Members’ individual contributions

- Elias Vera-Jimenez
    - Implementing the worker threads.
    - Implementing the logging system.
    - Testing the server and client code.
    - Answering parallelism question in README

- Hongzheng Li
    - Implementing the request queue.
    - Implementing the dispatcher threads.
    - Implementing the error handling system.
    - Implementing the cleanup system.

### Any changes you made to the Makefile or existing files that would affect grading

- Adding `make zip` to the Makefile

- Code styling changes on given files.

### Any assumptions that you made that weren’t outlined in section 7

- We assumed that the server would terminate after receiving `SIGINT`, `SIGTERM`,
`SIGQUIT`, `SIGHUP` signals, and all other signals would caused the server to terminate abnormally.

### How could you enable your program to make EACH individual request parallelized? (high-level pseudocode would be acceptable/preferred for this part)

To make each individual request parallelized, we would have to change the program to make it so the workers become their own processes instead of being threads. in this way, the program would become multi-process program that utilizes mutliple system cores rather than being one multi-threaded process. it would be structured like:

```pseudocode
# IN server
... 
dispatch ():
    while not interrupted:
        client = accept_connection()
        image = get_request(client)
        
        worker_id = execute worker with parameter image
        match = retrieve_result_from_worker(worker_id)
        send_file_to_client(match, client)
    exit server
... # 

# IN worker
database_arr[MAX_DATABASE_SIZE];
database_size;

load_database () : # Same as in server, load_database would be removed from server since database elements are mostly used in worker
    open database/
    for image in database/ :
        get image from database/
        store image in database_arr[database_size]
        database_size+1

image_match(image) :
    for i in database_size:
        if image matches database[i]:
            send_result_to_server(database[i])
            exit worker
```










