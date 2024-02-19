#include "ocm.h"

#include <stdlib.h>
#include <assert.h>

int64_t number_of_crossings(graph g, int u, int v)
{
    if (u == v)
        return 0;

    int64_t crossings = 0;

    int i = g.V[u];
    int j = g.V[v];

    while (i < g.V[u + 1] && j < g.V[v + 1])
    {
        if (g.E[j] < g.E[i])
        {
            crossings += g.V[u + 1] - i;
            j++;
        }
        else
            i++;
    }

    crossings *= (g.twins[u] + 1) * (g.twins[v] + 1);

    return crossings;
}

void lift_solution_twins(graph g, graph r, int **s)
{
    int *sl = malloc(sizeof(int) * g.B);

    int p = 0;
    for (int i = 0; i < r.B; i++)
    {
        int u = r.old_label[(*s)[i]];
        sl[p++] = u;
        if (r.twins[(*s)[i]] > 0)
        {
            int w = g.E[g.V[u]];
            for (int j = g.V[w]; j < g.V[w + 1]; j++)
            {
                int v = g.E[j];
                if (u != v && test_twin(g, u, v))
                    sl[p++] = v;
            }
        }
    }

    free(*s);
    *s = sl;
}

void lift_solution_degree_zero(graph g, graph r, int **s)
{
    int *sl = malloc(sizeof(int) * g.B);

    int p = 0;
    for (int i = 0; i < r.B; i++)
        sl[p++] = r.old_label[(*s)[i]];

    for (int u = g.A; u < g.N; u++)
        if (g.V[u + 1] - g.V[u] < 1)
            sl[p++] = u;

    free(*s);
    *s = sl;
}

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
            cm[u][v] = number_of_crossings(g, u + g.A, v + g.A);
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

int tc_add_independent(int **tc, int **cm, int N)
{
    int cost = 0;
    for (int i = 0; i < N; i++)
    {
        int c = 0;
        for (int j = 0; j < N; j++)
        {
            if (i == j)
                continue;

            if (tc[i][j] || tc[j][i])
                c++;
        }

        if (c == N - 2)
        {
            for (int j = 0; j < N; j++)
            {
                if (i == j)
                    continue;

                if (!tc[i][j] && !tc[j][i])
                {
                    if (cm[i][j] < cm[j][i])
                        cost += tc_add_edge(tc, tc, N, i, j, cm);
                    else
                        cost += tc_add_edge(tc, tc, N, j, i, cm);
                }
            }
        }
    }

    return cost;
}

int tc_try_add_edge(int **tc, int N, int u, int v, int **cm)
{
    int crossings = 0;

    for (int i = 0; i < N; i++)
    {
        if (i != u && tc[i][u] == 0)
            continue;

        for (int j = 0; j < N; j++)
        {
            if (j == i || (j != v && tc[v][j] == 0) || tc[i][j] == 1)
                continue;

            crossings += cm[i][j];
        }
    }

    return crossings;
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