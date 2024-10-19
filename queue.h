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

Queue* initQueue();
void pushQueue(Queue *queue, void *data);
void popQueue(Queue *queue);

#endif // QUEUE_H