#ifndef __QUEUE_H__
#define __QUEUE_H__

#define MAXLENGTH 100

struct Queue {
	char *pool[MAXLENGTH + 1];
	int start;
	int end;
	int size;
};

void initqueue(struct Queue *q);
void clearqueue(struct Queue *q);
int enqueue(struct Queue *q, char *str);
int dequeue(struct Queue *q);
char *queryqueue(const struct Queue *q, int index);
int queuesize(const struct Queue *q);
int queuefull(const struct Queue *q);

#endif
