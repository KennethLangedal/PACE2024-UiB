#include "ocm.h"

#include <stdlib.h>
#include <assert.h>

int **init_cost_matrix(graph g)
{
    int *data = malloc(sizeof(int) * g.B * g.B);
    int **cm = malloc(sizeof(int *) * g.B);

    for (int i = 0; i < g.B; i++)
        cm[i] = data + i * g.B;

    for (int u = 0; u < g.B; u++)
    {
        for (int v = 0; v < g.B; v++)
        {
            cm[u][v] = 0;
            if (u == v)
                continue;

            int i = g.V[g.A + u];
            int j = g.V[g.A + v];

            while (i < g.V[g.A + u + 1] && j < g.V[g.A + v + 1])
            {
                if (g.E[j] < g.E[i])
                {
                    cm[u][v] += g.V[g.A + u + 1] - i;
                    j++;
                }
                else
                    i++;
            }
        }
    }

    return cm;
}

void free_cost_matrix(int **cm)
{
    free(*cm);
    free(cm);
}

static inline int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int **init_tc(int N)
{
    int *data = malloc(sizeof(int) * N * N);
    int **tc = malloc(sizeof(int *) * N);

    for (int i = 0; i < N; i++)
        tc[i] = data + i * N;

    for (int i = 0; i < N * N; i++)
        data[i] = 0;

    return tc;
}

void tc_add_trivial(int **tc, int **cm, int N)
{
    for (int i = 0; i < N; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            if (cm[i][j] == 0)
                tc[i][j] = 1;
            else if (cm[j][i] == 0)
                tc[j][i] = 1;
        }
    }
}

int tc_add_edge(int **tc, int **tc_next, int N, int u, int v, int **cm)
{
    for (int i = 0; i < N * N; i++)
        (*tc_next)[i] = (*tc)[i];

    int crossings = 0;

    for (int i = 0; i < N; i++)
    {
        if (i != u && tc_next[i][u] == 0)
            continue;

        for (int j = 0; j < N; j++)
        {
            if (j == i || (j != v && tc_next[v][j] == 0) || tc_next[i][j] == 1)
                continue;

            tc_next[i][j] = 1;
            crossings += cm[i][j];
        }
    }

    return crossings;
}

int **free_tc(int **tc)
{
    free(*tc);
    free(tc);
}