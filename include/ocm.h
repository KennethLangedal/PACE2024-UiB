#pragma once
#include <stdio.h>

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