#pragma once

#include "ocm.h"

#include <stdio.h>

typedef struct
{
    int N;
    int *V, *E, *W;

    int *id_V, *id_E;
} dfas;

dfas dfas_construct_instance(ocm p);

dfas dfas_construct_subgraph(dfas p, int *mask);

void dfas_free(dfas p);

dfas *dfas_scc_split(dfas p, int *N);

int *dfas_solve(dfas p, int *c);

// int *dfas_solve_closed(dfas p, int *mask, int c1, int *c2);
