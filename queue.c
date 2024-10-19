#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} Queue;

Queue* initQueue() {
    Queue * queue = malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;

    return queue;
}

void pushQueue(Queue *queue, void *data) {
    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;

    if (queue->tail == NULL) {
        queue->head = node;
    } else {
        queue->tail->next = node;
    }

    queue->tail = node;
}

void popQueue(Queue *queue) {
    if (queue->head == NULL) {
        printf("Queue is empty\n");
    } else {
        Node* temp = queue->head;
        queue->head = queue->head->next;
        free(temp);
    }
}