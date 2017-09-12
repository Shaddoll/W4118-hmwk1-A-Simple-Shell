#ifndef __QUEUE_H__
#define __QUEUE_H__

typedef struct Queue Queue;

void initqueue(Queue* q);
void clearqueue(Queue* q);
int enqueue(Queue* q, char *str);
int dequeue(Queue* q);
char* queryqueue(Queue* q, int index);

#endif
