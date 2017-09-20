#ifndef __Graph_H__
#define __Graph_H__

#define MAXVERTEX 100

struct Graph {
	int edges[MAXVERTEX][MAXVERTEX];
	int start;
};

void initgraph(struct Graph *g);
void cleargraph(struct Graph *g);
void shiftvertex(struct Graph *g);
void setedge(struct Graph *g, int from, int to);
int checkcycle(const struct Graph *g, int vertex);

#endif
