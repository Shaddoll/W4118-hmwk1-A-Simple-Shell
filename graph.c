#include "Graph.h"

void initgraph(Graph *g) {
        int i, j;
        for (i = 0; i < MAXVERTEX; ++i) {
                for (j = 0; j < MAXVERTEX; ++j) {
                    g->edges[i][j] = 0;
                }
        }
        g->start = 0;
}

void cleargraph(Graph *g) {
        initgraph(g);
}

void shiftvertex(Graph *g) {
        int i;
        for (i = 0; i < MAXVERTEX; ++i) {
                g->edges[g->start][i] = 0;
        }
        g->start = (g->start + 1) % MAXVERTEX;
}

void setedge(Graph *g, int from, int to) {
        if (from < 0 || from >= MAXVERTEX || to < 0 || to >= MAXVERTEX) {
                return;
        }
        g->edges[(g->start + from) % MAXVERTEX][to] = 1;
}

void dfs(const Graph *g, int *flag, int vertex, int *cycle) {
        int i;
        if (flag[(g->start + vertex) % MAXVERTEX]) {
                *cycle = 1;
        }
        flag[(g->start + vertex) % MAXVERTEX] = 1;
        for (i = 0; i < MAXVERTEX; ++i) {
                if (*cycle) {
                        return;
                }
                if (g->edges[(g->start + vertex) % MAXVERTEX][i]) {
                    dfs(g, flag, i, cycle);
                }
        }
        flag[(g->start + vertex) % MAXVERTEX] = 0;
}

int checkcycle(const Graph *g, int vertex) {
        int cycle = 0;
        int flag[MAXVERTEX] = {0};
        dfs(g, flag, vertex, &cycle);
        return cycle;
}

