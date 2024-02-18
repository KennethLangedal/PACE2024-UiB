#pragma once

#include "graph.h"

int number_of_crossings(graph g, int u, int v);

int *lift_solution_degree_one(graph g, graph r, int *s);

int **init_cost_matrix(graph g);

void free_cost_matrix(int **cm);

int **init_tc(int N);

void tc_add_trivial(int **tc, int **cm, int N);

void tc_add_independent(int **tc, int **cm, int N);

int tc_try_add_edge(int **tc, int N, int u, int v, int **cm);

int tc_add_edge(int **tc, int **tc_next, int N, int u, int v, int **cm);

int **free_tc(int **tc);