#pragma once

#include "graph.h"
#include "ocm.h"
#include "dfas.h"

int dfas_simulated_annealing(dfas p, int *s);

void ocm_simulated_annealing(ocm *p);

int simulated_annealing(graph g, int *s);