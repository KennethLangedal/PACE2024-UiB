#pragma once
#include "ocm.h"

typedef struct
{
    int n;   // Vertices in component
    int **W; // Adjacency matrix
    int *c;  // Current solution cost
    int *S;  // Current solution
    int *I;  // Old ID
} comp;

typedef struct
{
    int n;      // Number of strongly connected components
    int offset; // Number of unavoidable crossings
    int *O;     // Vertex ordering based on ranges
    comp *C;    // Components
} dfas;

dfas dfas_construct(ocm p);

void dfas_free(dfas g);

comp dfas_construct_subgraph(comp c, int *E, int m);

void dfas_free_comp(comp c);

int *dfas_get_solution(ocm p, dfas g);
