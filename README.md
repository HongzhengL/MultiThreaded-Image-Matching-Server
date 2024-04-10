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

### Any changes you made to the Makefile or existing files that would affect grading

- Adding `make zip` to the Makefile

- Code styling changes on given files.

### Any assumptions that you made that weren’t outlined in section 7

- We assumed that the server would terminate after receiving `SIGINT`, `SIGTERM`,
`SIGQUIT`, `SIGHUP` signals, and all other signals would caused the server to terminate abnormally.

### How could you enable your program to make EACH individual request parallelized? (high-level pseudocode would be acceptable/preferred for this part)

