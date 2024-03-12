#pragma once

#include <stdio.h>

typedef struct
{
    int N, A, B;
    int *V, *E;

    int *twins;
    int *old_label;
} graph;

graph parse_graph(FILE *f);

void store_graph(FILE *f, graph g);

int test_twin(graph g, int u, int v);

graph subgraph(graph g, int *mask);

graph remove_degree_zero(graph g);

graph remove_twins(graph g);

graph *split_graph(graph g, int *N);

void free_graph(graph g);

int validate_graph(graph g);