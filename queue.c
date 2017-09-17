#include <stdlib.h>
#include <string.h>
#include "queue.h"

void initqueue(Queue *q)
{
	int i;

	for (i = 0; i < MAXLENGTH + 1; ++i)
		q->pool[i] = NULL;
	q->start = 0;
	q->end = 0;
	q->size = 0;
}

void clearqueue(Queue *q)
{
	int i;

	for (i = q->start; i < q->end; ++i) {
		free(q->pool[i]);
		q->pool[i] = NULL;
		if (i == MAXLENGTH)
			i = -1;
	}
	q->start = 0;
	q->end = 0;
	q->size = 0;
}

int enqueue(Queue *q, char *str)
{
	size_t len;

	if (q->size == MAXLENGTH)
		return -1;
	len = strlen(str) + 1;
	q->pool[q->end] = malloc(len * sizeof(char));
	if (q->pool[q->end] == NULL)
		return -1;
	strcpy(q->pool[q->end], str);
	q->end = (q->end + 1) % (MAXLENGTH + 1);
	return ++q->size;
}

int dequeue(Queue *q)
{
	if (q->size == 0)
		return -1;
	free(q->pool[q->start]);
	q->pool[q->start] = NULL;
	q->start = (q->start + 1) % (MAXLENGTH + 1);
	return --q->size;
}

char *queryqueue(const Queue *q, int index)
{
	if (index >= q->size || index < 0)
		return NULL;
	return q->pool[(q->start + index) % (MAXLENGTH + 1)];
}

int queuesize(const Queue *q)
{
	return q->size;
}

int queuefull(const Queue *q)
{
	return q->size == MAXLENGTH;
}
