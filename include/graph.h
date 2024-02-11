#pragma once

#include <stdio.h>

void graph_parse(FILE *f, int *N, int *N0, int *N1, int **V, int **E);

int graph_validate(int N, int N0, int N1, const int *V, const int *E);