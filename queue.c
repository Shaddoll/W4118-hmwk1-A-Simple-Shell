#include <stdlib.h>
#include <string.h>
#include "queue.h"

#define MAXLENGTH 100

struct Queue {
        char *pool[MAXLENGTH + 1];
        int start;
        int end;
        int size;
};

void initqueue(Queue* q) {
        int i;
        for (i = 0; i < MAXLENGTH + 1; ++i) {
                q->pool[i] = NULL;
        }
        q->start = 0;
        q->end = 0;
        q->size = 0;
}

void clearqueue(Queue* q) {
        int i;
        for (i = q->start; i < q->end; ++i) {
                free(q->pool[i]);
                q->pool[i] = NULL;
                if (i == MAXLENGTH) {
                    i = -1;
                }
        }
        q->start = 0;
        q->end = 0;
        q->size = 0;
}

int enqueue(Queue* q, char *str) {
        if (q->size == MAXLENGTH) {
                return -1;
        }
        size_t len = strlen(str);
        q->pool[q->end] = malloc(len * sizeof(char));
        if (q->pool[q->end] == NULL) {
                return -1;
        }
        strcpy(q->pool[q->end], str);
        q->end = (q->end + 1) % (MAXLENGTH + 1);
        return ++q->size;
}

int dequeue(Queue* q) {
        if (q->size == 0) {
                return -1;
        }
        free(q->pool[q->start]);
        q->pool[q->start] = NULL;
        q->start = (q->start + 1) % (MAXLENGTH + 1);
        return --q->size;
}

char *queryqueue(Queue* q, int index) {
        if (index >= q->size) {
                return NULL;
        }
        return q->pool[(q->start + index) % (MAXLENGTH + 1)];
}
