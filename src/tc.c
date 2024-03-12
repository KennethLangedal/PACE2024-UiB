#include "tc.h"

#include <stdlib.h>
#include <immintrin.h>

#include <stdio.h>

tc *tc_init(int N)
{
    tc g = {.N = N,
            .d0 = 0,
            .d1 = 0,
            .P = malloc(sizeof(int *) * N),
            .W = malloc(sizeof(int *) * N),
            .I = malloc(sizeof(int) * N * N),
            .J = malloc(sizeof(int) * N * N),
            .cost = malloc(sizeof(int) * N * N)};

    int *p = malloc(sizeof(int) * N * N);
    int *w = malloc(sizeof(int) * N * N);

    for (int i = 0; i < N * N; i++)
        p[i] = 0;

    for (int i = 0; i < N; i++)
        p[i + i * N] = 1;

    for (int i = 0; i < N * N; i++)
        w[i] = 0;

    for (int i = 0; i < N; i++)
        g.P[i] = p + i * N;

    for (int i = 0; i < N; i++)
        g.W[i] = w + i * N;

    g.cost[0] = 0;

    tc *res = malloc(sizeof(tc));
    *res = g;

    return res;
}

void tc_free(tc *g)
{
    free(*g->P);
    free(*g->W);
    free(g->P);
    free(g->W);

    free(g->I);
    free(g->J);
    free(g->cost);
}

void tc_add_edge(tc *g, int u, int v)
{
    g->d0++;
    g->cost[g->d0] = g->cost[g->d0 - 1];
    for (int i = 0; i < g->N; i++)
    {
        if (!g->P[i][u])
            continue;

        for (int j = 0; j < g->N; j++)
        {
            if (g->P[i][v])
                continue;

            g->I[g->d1] = i;
            g->J[g->d1] = j;
            g->d1++;

            g->P[i][j] = 1;
            g->cost[g->d0] += g->W[i][j];
        }
    }
}

void tc_pop_edge(tc *g, int u, int v)
{
    g->d0--;
    int i, j;
    do
    {
        g->d1--;
        int i = g->I[g->d1], j = g->J[g->d1];
        g->P[i][j] = 0;
    } while (i != u && j != v);
}

int tc_cost(tc *g)
{
    return g->cost[g->d0];
}