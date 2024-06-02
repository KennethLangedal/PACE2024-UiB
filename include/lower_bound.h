#pragma once
#include "dfas.h"

int lower_bound_greedy(comp c);

int lower_bound_flow_check(comp c);

int lower_bound_cycle_packing(comp c);

void lower_bound_cycle_packing_lp(comp c);