#include "../include/server.h"

struct request_node {
    request_t *request;
    struct request_node *next;
};

struct request_node *head;
struct request_node *tail;
int queue_len;

int main() {
    printf("starting\n");
    queue_len = 0;
    tail = malloc(sizeof(struct request_node));
    tail->request = (request_t *)malloc(sizeof(request_t));
    tail->next = malloc(sizeof(struct request_node));
    head = tail;

    for (int i = 0; i < 10; i++) {
        printf("I = %d\n", i);
        request_t *request = (request_t *)malloc(sizeof(request_t *));
        printf("request initialized\n");

        char* buffer = malloc(strlen("Hello ") + sizeof(int) +1 * sizeof(char));
        sprintf(buffer, "Hello %d", i);
        printf("buffer = %s, size of buffer = %ld\n", buffer, sizeof(buffer));

        request->buffer = malloc(sizeof(buffer)+1);
        strncpy(request->buffer, buffer, sizeof(request->buffer) - 1);
        request->buffer[sizeof(request->buffer) - 1] = '\0';
        
        request->file_size = strlen("Hello ") + sizeof(int) + 1; //+1 for null pointer
        printf("request->buffer = %s, request->file_size %d\n", request->buffer, request->file_size);
        struct request_node *req_node = malloc(sizeof(struct request_node));

        req_node->request = (request_t *)malloc(sizeof(request));
        *req_node->request = *request;
        free(request);
        printf("req_node->request->buffer = %s\n", req_node->request->buffer);
        req_node->next = malloc(sizeof(struct request_node));
        *tail->next = *req_node;
        tail = tail->next;
        
        
        free(req_node);
    }

    for (int j = 0; j < 10; j++) {
        struct request_node *curr_node = head->next;
        printf("%s, size = %d\n",curr_node->request->buffer, curr_node->request->file_size);
        struct request_node *temp = head;
        head = curr_node;
        free(temp);
    }

    printf("finished\n");
}