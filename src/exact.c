#include "exact.h"
#include "ipamir.h"
#include "cycle_packing.h"

#include <stdlib.h>

void lazy_add_length_3(void *solver, int **D, int n, int *C)
{
    for (int u = 0; u < n; u++)
    {
        for (int v = u + 1; v < n; v++)
        {
            if (D[u][v] == 0)
                continue;
            for (int w = u + 1; w < n; w++)
            {
                if (w != v && D[u][v] > 0 && D[v][w] > 0 && D[w][u] > 0)
                {
                    ipamir_add_hard(solver, (u * n + v) + 1);
                    ipamir_add_hard(solver, (v * n + w) + 1);
                    ipamir_add_hard(solver, (w * n + u) + 1);
                    ipamir_add_hard(solver, 0);
                    (*C)++;
                }
            }
        }
    }
}

void lazy_add_length_4(void *solver, int **D, int n, int *C)
{
    for (int u = 0; u < n; u++)
    {
        for (int v = u + 1; v < n; v++)
        {
            if (D[u][v] == 0)
                continue;
            for (int w = u + 1; w < n; w++)
            {
                if (w == v || D[v][w] == 0)
                    continue;
                for (int x = u + 1; x < n; x++)
                {
                    if (x != v && x != w && D[w][x] > 0 && D[x][u] > 0)
                    {
                        // if ((rand() % 100) > 0)
                        //     continue;
                        ipamir_add_hard(solver, (u * n + v) + 1);
                        ipamir_add_hard(solver, (v * n + w) + 1);
                        ipamir_add_hard(solver, (w * n + x) + 1);
                        ipamir_add_hard(solver, (x * n + u) + 1);
                        ipamir_add_hard(solver, 0);
                        (*C)++;

                        if (*C > 50000000)
                            return;
                    }
                }
            }
        }
    }
}

int solve_lazy(comp c)
{
    const char *name = ipamir_signature();
    // fprintf(stderr, "Using: %s\n", name);

    void *solver = ipamir_init();
    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            if (c.W[i][j] > 0)
                ipamir_add_soft_lit(solver, (i * c.n + j) + 1, c.W[i][j]);

    int *data = malloc(sizeof(int) * c.n * c.n);
    int **D = malloc(sizeof(int *) * c.n);
    for (int i = 0; i < c.n; i++)
        D[i] = data + i * c.n;

    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            D[i][j] = c.W[i][j];

    int cost = *c.c - 1, C = 0, old_C = -1;
    lazy_add_length_3(solver, D, c.n, &C);
    lazy_add_length_4(solver, D, c.n, &C);

    while (cost < *c.c)
    {
        if (old_C == C)
            break;

        // fprintf(stderr, "Solving %d hard\n", C);

        ipamir_solve(solver);
        cost = (int)ipamir_val_obj(solver);

        // fprintf(stderr, "Solved %d vs %d\n", *c.c, cost);

        if (cost == *c.c)
            break;

        for (int i = 0; i < c.n; i++)
            for (int j = 0; j < c.n; j++)
                if (c.W[i][j] > 0)
                    D[i][j] = ipamir_val_lit(solver, (i * c.n + j) + 1) >= 0 ? 0 : c.W[i][j];

        packing p = cycle_packing_init(D, c.n);
        cycle_packing(p);

        old_C = C;
        for (int u = 0; u < c.n; u++)
        {
            for (int v = 0; v < c.n; v++)
            {
                int uv = u * c.n + v;
                for (int i = p.V[uv]; i < p.V[uv + 1]; i++)
                {
                    if (p.edges[i] == NULL || p.edges[i]->e[0] != uv || p.edges[i]->n < 4)
                        continue;

                    cycle *c = p.edges[i];
                    for (int j = 0; j < c->n; j++)
                        ipamir_add_hard(solver, c->e[j] + 1);

                    ipamir_add_hard(solver, 0);
                    C++;
                }
            }
        }
        cycle_packing_free(p);
        if (old_C == C)
            break;
    }

    if (*c.c != cost)
    {
        // Fix solution
        *c.c = cost;
        int *in_degree = malloc(sizeof(int) * c.n);
        for (int i = 0; i < c.n; i++)
            in_degree[i] = 0;

        for (int u = 0; u < c.n; u++)
            for (int v = 0; v < c.n; v++)
                if (D[u][v] > 0)
                    in_degree[v]++;

        int s = 0;
        while (s < c.n)
        {
            // Find sources
            for (int u = 0; u < c.n; u++)
            {
                if (in_degree[u] == 0)
                {
                    in_degree[u] = -1;
                    c.S[s++] = u;
                    for (int v = 0; v < c.n; v++)
                        if (D[u][v] > 0)
                            in_degree[v]--;
                }
            }
        }

        free(in_degree);
    }

    free(D);
    free(data);
    ipamir_release(solver);
    return 1;
}