#pragma once
#include "dfas.h"

typedef struct
{
    int n, w;
    int *e, *p;
} cycle;

typedef struct
{
    int n, _n, m;
    int *V, *C, *c;
    cycle **edges;
} packing;

packing cycle_packing_init(comp c);

void cycle_packing_free(packing p);

void cycle_packing_greedy(packing p);