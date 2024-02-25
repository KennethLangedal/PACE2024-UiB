#pragma once

#include "graph.h"

#include <stdint.h>

typedef struct
{
    int N;
    int **cm, **tc;
    int crossings, lb;
    int undecided, equal;
} ocm;

ocm init_ocm_problem(graph g);

void free_ocm_problem(ocm p);

// Reduction rules

void ocm_reduction_trivial(ocm *p);

void ocm_reduction_2_1(graph g, ocm *p);

void ocm_reduction_twins(graph g, ocm *p);

void ocm_reduction_independent(ocm *p);

void ocm_reduction_k_quick(ocm *p, int ub);

void ocm_reduction_k_full(ocm *p, int ub);

// B&B tools

// Returns the number of extra crossings above lb
int ocm_try_edge(ocm p, int u, int v);

void ocm_add_edge(ocm *p, int u, int v);

// Util

int64_t count_crossings_pair(graph g, int u, int v);

int64_t count_crossings_solution(graph g, int *s);

void lift_solution_degree_zero(graph g, graph r, int **s);

int count_relevant_vertices(ocm p);

void ocm_copy_tc(ocm s, ocm *d);

void ocm_copy_full(ocm s, ocm *d);