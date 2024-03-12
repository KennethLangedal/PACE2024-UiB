#pragma once

#include "ocm.h"

#include <stdio.h>

typedef struct
{
    int N;
    int *V, *E, *W, *a;
    int *cc, *cc_size, *n_cc;
} dfas;

dfas dfas_construct_instance(ocm p);

void dfas_free(dfas p);

void dfas_reduction_cc(dfas p);

void dfas_reduction_degree_one(dfas p);

void dfas_store_dfvs(FILE *f, dfas p);

void dfas_solve(dfas p);

void dfas_solve_cc(dfas p);

// typedef struct
// {
//     int N, Nc, W, old_id, in, a;
//     int *C;
// } var;

// typedef struct
// {
//     int N, count, sat;
//     int *V;
// } constraint;

// typedef struct
// {
//     int v, opt;
// } log;

// typedef struct
// {
//     int N, M, am;
//     int best_cost, *best;
//     int *new_id;

//     var *V;
//     constraint *C;

//     int cost;
//     int H;
//     log *history;
// } dfas_p;

// dfas_p init_dfas_p(dfas p, int cc);

// void free_dfas_p(dfas_p p);

// void add_cycles(dfas_p *p, dfas g, int cc, int use_p);

// void solve(dfas_p *p);

// int validate(dfas_p p);