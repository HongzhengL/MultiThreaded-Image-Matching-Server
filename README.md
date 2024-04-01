# CSCI 4061 Project 3

## MultiThreaded Image Matching Server

### Project group number

- Group 44

### Group member names and x500s

- Elias Vera-Jimenez (veraj002)
- Hongzheng Li (li003458)

### The name of the CSELabs computer that you tested your code on

- csel-kh1250-13.cselabs.umn.edu

### Any changes you made to the Makefile or existing files that would affect grading

- Adding `make zip`, `make run`, `make kill` to the Makefile.

- Code styling changes on given files.

### Plan outlining individual contributions for each member of your group

- Elias Vera-Jimenez
  - Implementing the worker threads.
  - Implementing the dispatcher threads.
  - Implementing the logging system.
  - Implementing the error handling system.
  - Testing the server and client code.

- Hongzheng Li
    - Implementing the request queue.
    - Implementing the image matching algorithm.
    - Implementing the signal handling system.
    - Implementing the cleanup system.
    - Implementing Client and Server Communication

### Plan on how you are going to construct the worker threads and how you will make use of mutex locks and condition variables

- Create `num_worker` of worker threads at server startup using `pthread_create()`
- Implement a loop within each worker thread to continuously check for new requests.
- Use a mutex lock to protect access to the shared request queue.
- If the queue is empty, worker threads should wait on a condition variable until a new request is added.
- Upon receiving a signal, a thread locks the mutex, processes a request from the queue, and then unlocks the mutex.
- After adding a request to the queue, dispatcher threads signal a condition variable to wake up a waiting worker thread.