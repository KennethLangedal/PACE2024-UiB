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

int *dfas_get_solution(ocm p, dfas g);

// typedef struct
// {
//     int n, offset;
//     int *V, *E, *W;

//     int *ID_V, *ID_E;
// } dfas;

// typedef struct
// {
//     int n;
//     dfas *C;
// } scc_split;

// dfas dfas_construct(ocm p, int max_edges);

// dfas dfas_transpose(dfas g);

// void dfas_free(dfas g);

// void dfas_free_scc_split(scc_split cc);

// dfas dfas_construct_subgraph(dfas g, int *mask);

// scc_split dfas_scc_split(dfas g);

// void dfas_lift_scc(dfas g, int *Sc, int *S);

// int *dfas_lift_solution(ocm p, dfas g, int *S);

// int dfas_eval_solution(ocm p, int *O);