#pragma once

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

packing cycle_packing_init(int **W, int n);

void cycle_packing_free(packing p);

void cycle_packing(packing p);
