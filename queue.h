#ifndef __QUEUE_H__
#define __QUEUE_H__

#define MAXLENGTH 100

typedef struct Queue {
        char *pool[MAXLENGTH + 1];
        int start;
        int end;
        int size;
} Queue;

void initqueue(Queue *q);
void clearqueue(Queue *q);
int enqueue(Queue *q, char *str);
int dequeue(Queue *q);
char *queryqueue(const Queue *q, int index);
int queuesize(const Queue *q);
int queuefull(const Queue *q);

#endif
