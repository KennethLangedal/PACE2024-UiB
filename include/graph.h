#pragma once

#include <stdio.h>

typedef struct
{
    int N, A, B;
    int *V, *E;

    int *old_label, *twins;
} graph;

graph parse_graph(FILE *f);

graph subgraph(graph g, int *mask);

graph remove_degree_one(graph g);

graph *split_graph(graph g, int *N);

graph remove_twins(graph g);

void free_graph(graph g);

int validate_graph(graph g);