#pragma once
#include "dfas.h"

#include <signal.h>

void heuristics_greedy_improvement(comp c);

void heuristics_greedy_cut(comp c, volatile sig_atomic_t *tle);

void heuristic_randomize_solution(comp c, int changes);

// void heuristic_greedy_direct(dfas p, int *S);