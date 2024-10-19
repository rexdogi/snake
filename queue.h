#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} Queue;

// Declaration of functions
Queue* initQueue();  // Ensure no 'extern' keyword is here
void pushQueue(Queue *queue, void *data);
void popQueue(Queue *queue);

#endif // QUEUE_H