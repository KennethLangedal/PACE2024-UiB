#pragma once

#include <stdio.h>

typedef struct
{
    int N, A, B;
    int *V, *E;

    int *old_label, *twins;
} graph;

graph parse_graph(FILE *f);

void store_graph(FILE *f, graph g);

graph subgraph(graph g, int *mask);

graph remove_degree_one(graph g);

graph *split_graph(graph g, int *N);

int test_twin(graph g, int u, int v);

graph remove_twins(graph g);

void free_graph(graph g);

int validate_graph(graph g);