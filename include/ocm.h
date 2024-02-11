#pragma once

int **ocm_compute_cost_matrix(int N, const int *V, const int *E, int N0, int N1);

void ocm_free_cost_matrix(int **cm);