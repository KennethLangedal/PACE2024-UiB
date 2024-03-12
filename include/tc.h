#pragma once

typedef struct
{
    int N, d0, d1;
    int **P, **W;
    int *I, *J, *cost;
} tc;

tc *tc_init(int N);

void tc_free(tc *g);

void tc_add_edge(tc *g, int u, int v);

void tc_pop_edge(tc *g, int u, int v);

int tc_cost(tc *g);