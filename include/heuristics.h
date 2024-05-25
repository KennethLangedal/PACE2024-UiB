#pragma once
#include "dfas.h"

void heuristics_greedy_improvement(comp c);

void heuristics_greedy_cut(comp c);

void heuristic_randomize_solution(comp c, int changes);

// int *heuristics_greedy(dfas g, int max_it, volatile sig_atomic_t *term);

// int *heuristics_random_local_search(dfas g, int max_it, volatile sig_atomic_t *term);

// int *heuristics_token_swapping(dfas g, dfas gt, int max_it, volatile sig_atomic_t *term);