#pragma once
#include <stdio.h>
#include <signal.h>

typedef struct
{
    int n0, n1, m;
    int *V, *E;

    int cw;
    int *C;
} ocm;

ocm ocm_parse(FILE *f);

void ocm_free(ocm p);

int ocm_validate(ocm p);

int *ocm_average_placement(ocm p);

void ocm_greedy_improvement(ocm p, int *S, int *c, volatile sig_atomic_t *tle);