#ifndef __Graph_H__
#define __Graph_H__

#define MAXVERTEX 100

typedef struct Graph {
        int edges[MAXVERTEX][MAXVERTEX];
        int start;
} Graph;

void initgraph(Graph *g);
void shiftvertex(Graph *g);
void setedge(Graph *g, int from, int to);
int checkcycle(const Graph *g, int vertex);

#endif
