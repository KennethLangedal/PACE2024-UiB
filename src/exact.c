#include "exact.h"
#include "ipamir.h"

#include <stdlib.h>

int solve(comp c)
{
    const char *name = ipamir_signature();
    fprintf(stderr, "Using: %s\n", name);

    void *solver = ipamir_init();
    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            if (c.W[i][j] > 0)
                ipamir_add_soft_lit(solver, i * c.n + j, c.W[i][j]);

    int C = 0;
    for (int i = 0; i < c.n; i++)
    {
        int u = c.S[i];
        for (int j = i + 1; j < c.n; j++)
        {
            int v = c.S[j];
            if (c.W[v][u] == 0)
                continue;

            int n = 0;
            for (int k = i + 1; k < j; k++)
            {
                int w = c.S[k];
                if (c.W[u][w] == 0)
                    continue;

                if (c.W[w][v] > 0)
                {
                    ipamir_add_hard(solver, u * c.n + w);
                    ipamir_add_hard(solver, w * c.n + v);
                    ipamir_add_hard(solver, v * c.n + u);
                    ipamir_add_hard(solver, 0);
                    C++;
                    continue;
                }

                for (int l = k + 1; l < j; l++)
                {
                    int x = c.S[l];
                    if (c.W[w][x] == 0)
                        continue;

                    if (c.W[x][v] > 0)
                    {
                        ipamir_add_hard(solver, u * c.n + w);
                        ipamir_add_hard(solver, w * c.n + x);
                        ipamir_add_hard(solver, x * c.n + v);
                        ipamir_add_hard(solver, v * c.n + u);
                        ipamir_add_hard(solver, 0);
                        C++;
                        continue;
                    }

                    for (int m = l + 1; m < j; m++)
                    {
                        int y = c.S[m];
                        if (c.W[x][y] == 0)
                            continue;

                        if (c.W[y][v] > 0)
                        {
                            ipamir_add_hard(solver, u * c.n + w);
                            ipamir_add_hard(solver, w * c.n + x);
                            ipamir_add_hard(solver, x * c.n + y);
                            ipamir_add_hard(solver, y * c.n + v);
                            ipamir_add_hard(solver, v * c.n + u);
                            ipamir_add_hard(solver, 0);
                            C++;
                            continue;
                        }

                        for (int n = m + 1; n < j; n++)
                        {
                            int z = c.S[n];
                            if (c.W[y][z] == 0)
                                continue;

                            if (c.W[z][v] > 0)
                            {
                                ipamir_add_hard(solver, u * c.n + w);
                                ipamir_add_hard(solver, w * c.n + x);
                                ipamir_add_hard(solver, x * c.n + y);
                                ipamir_add_hard(solver, y * c.n + z);
                                ipamir_add_hard(solver, z * c.n + v);
                                ipamir_add_hard(solver, v * c.n + u);
                                ipamir_add_hard(solver, 0);
                                C++;
                                continue;
                            }
                        }
                    }
                }
            }

            // printf("%d %d (%d %d) %d (%d)\n", u, v, i, j, n, min);
            // printf("Back edge: %d %d\n", c.W[v][u], V);
        }
    }

    fprintf(stderr, "Solving %d hard\n", C);
    ipamir_solve(solver);

    fprintf(stderr, "Solved %d vs %d\n", *c.c, (int)ipamir_val_obj(solver));

    ipamir_release(solver);

    // printf("Could be enough with %d cycles\n", V);

    return *c.c == (int)ipamir_val_obj(solver);
}

int solve_dual(comp c)
{
    const char *name = ipamir_signature();
    // fprintf(stderr, "Using: %s\n", name);

    void *solver = ipamir_init();
    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            if (c.W[i][j] > 0)
                ipamir_add_soft_lit(solver, i * c.n + j, c.W[i][j]);

    int C = 0;
    for (int u = 0; u < c.n; u++)
    {
        for (int v = u + 1; v < c.n; v++)
        {
            ipamir_add_hard(solver, u * c.n + v);
            ipamir_add_hard(solver, v * c.n + u);
            ipamir_add_hard(solver, 0);
            C++;

            ipamir_add_hard(solver, -(u * c.n + v));
            ipamir_add_hard(solver, -(v * c.n + u));
            ipamir_add_hard(solver, 0);
            C++;

            for (int w = u + 1; w < c.n; w++)
            {
                if (w == v)
                    continue;

                ipamir_add_hard(solver, u * c.n + v);
                ipamir_add_hard(solver, v * c.n + w);
                ipamir_add_hard(solver, w * c.n + u);
                ipamir_add_hard(solver, 0);
                C++;
            }
        }
    }

    // fprintf(stderr, "Solving %d hard\n", C);
    ipamir_solve(solver);

    // fprintf(stderr, "Solved %d vs %d\n", *c.c, (int)ipamir_val_obj(solver));
    int res = *c.c == (int)ipamir_val_obj(solver);

    ipamir_release(solver);

    // printf("Could be enough with %d cycles\n", V);

    return res;
}

void explore(void *solver, int **D, int **W, int *visited, int *on_stack, int *prev, int u, int n, int *C, int max)
{
    on_stack[u] = 1;
    visited[u] = 1;
    for (int v = 0; v < n; v++)
    {
        if (D[u][v] || W[u][v] == 0)
            continue;

        if (on_stack[v])
        {
            int _u = prev[u], _v = u;
            int len = 0;
            do
            {
                len++;
                _u = prev[_u];
                _v = prev[_v];
            } while (_v != v);
            if (len > max)
                continue;

            ipamir_add_hard(solver, u * n + v);
            _u = prev[u], _v = u;
            do
            {
                ipamir_add_hard(solver, _u * n + _v);
                _u = prev[_u];
                _v = prev[_v];
            } while (_v != v);
            ipamir_add_hard(solver, 0);
            *C += 1;
            continue;
        }

        if (visited[v])
            continue;

        prev[v] = u;
        explore(solver, D, W, visited, on_stack, prev, v, n, C, max);
    }
    on_stack[u] = 0;
}

void explore_packing(void *solver, int **D, int *visited, int *on_stack, int *prev, int u, int n, int *C, int *lb)
{
    on_stack[u] = 1;
    visited[u] = 1;
    for (int v = 0; v < n; v++)
    {
        if (D[u][v] == 0)
            continue;

        if (on_stack[v])
        {

            int _u = prev[u], _v = u, min = 99999;
            do
            {
                if (D[_u][_v] < min)
                    min = D[_u][_v];
                _u = prev[_u];
                _v = prev[_v];
            } while (_v != v);
            if (min == 0)
                continue;

            ipamir_add_hard(solver, u * n + v);
            *lb += min;
            _u = prev[u], _v = u;
            do
            {
                ipamir_add_hard(solver, _u * n + _v);
                D[_u][_v] -= min;
                _u = prev[_u];
                _v = prev[_v];
            } while (_v != v);
            ipamir_add_hard(solver, 0);
            *C += 1;

            continue;
        }

        if (visited[v])
            continue;

        prev[v] = u;
        explore_packing(solver, D, visited, on_stack, prev, v, n, C, lb);
    }
    on_stack[u] = 0;
}

void lazy_cycle_packing(void *solver, int **D, int n, int *visited, int *on_stack, int *prev, int *C, int *lb)
{
    for (int u = 0; u < n; u++)
    {
        for (int v = u + 1; v < n; v++)
        {
            for (int w = u + 1; w < n && D[u][v] > 0; w++)
            {
                if (w != v && D[u][v] > 0 && D[v][w] > 0 && D[w][u] > 0)
                {
                    int min = D[u][v] < D[v][w] ? D[u][v] : D[v][w];
                    min = min < D[w][u] ? min : D[w][u];
                    D[u][v] -= min;
                    D[v][w] -= min;
                    D[w][u] -= min;
                    *lb += min;
                    ipamir_add_hard(solver, u * n + v);
                    ipamir_add_hard(solver, v * n + w);
                    ipamir_add_hard(solver, w * n + u);
                    ipamir_add_hard(solver, 0);
                    (*C)++;
                }
            }
        }
    }

    for (int u = 0; u < n; u++)
    {
        for (int v = u + 1; v < n; v++)
        {
            for (int w = u + 1; w < n && D[u][v] > 0; w++)
            {
                if (w == v)
                    continue;

                for (int x = u + 1; x < n && D[v][w] > 0; x++)
                {
                    if (x != v && x != w && D[u][v] > 0 && D[v][w] > 0 && D[w][x] > 0 && D[x][u] > 0)
                    {
                        int min = D[u][v] < D[v][w] ? D[u][v] : D[v][w];
                        min = min < D[w][x] ? min : D[w][x];
                        min = min < D[x][u] ? min : D[x][u];
                        D[u][v] -= min;
                        D[v][w] -= min;
                        D[w][x] -= min;
                        D[x][u] -= min;
                        *lb += min;
                        ipamir_add_hard(solver, u * n + v);
                        ipamir_add_hard(solver, v * n + w);
                        ipamir_add_hard(solver, w * n + x);
                        ipamir_add_hard(solver, x * n + u);
                        ipamir_add_hard(solver, 0);
                        (*C)++;
                    }
                }
            }
        }
    }

    int old_lb = *lb + 1;
    while (old_lb > *lb)
    {
        old_lb = *lb;
        for (int i = 0; i < n; i++)
        {
            visited[i] = 0;
            on_stack[i] = 0;
            prev[i] = -1;
        }

        for (int u = 0; u < n; u++)
        {
            if (visited[u])
                continue;

            explore_packing(solver, D, visited, on_stack, prev, u, n, C, lb);
        }
    }
}

int solve_lazy(comp c)
{
    const char *name = ipamir_signature();
    fprintf(stderr, "Using: %s\n", name);

    void *solver = ipamir_init();
    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            if (c.W[i][j] > 0)
                ipamir_add_soft_lit(solver, i * c.n + j, c.W[i][j]);

    int *data = malloc(sizeof(int) * c.n * c.n);
    int **D = malloc(sizeof(int *) * c.n);
    for (int i = 0; i < c.n; i++)
        D[i] = data + i * c.n;

    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            D[i][j] = c.W[i][j];

    int *visited = malloc(sizeof(int) * c.n);
    int *on_stack = malloc(sizeof(int) * c.n);
    int *prev = malloc(sizeof(int) * c.n);

    int C = 0, lb = 0;
    lazy_cycle_packing(solver, D, c.n, visited, on_stack, prev, &C, &lb);


    fprintf(stderr, "Solving %d hard (lb=%d)\n", C, lb);
    ipamir_solve(solver);

    fprintf(stderr, "Solved %d vs %d\n", *c.c, (int)ipamir_val_obj(solver));

    int max = 3;

    while (*c.c != (int)ipamir_val_obj(solver))
    {
        for (int i = 0; i < c.n * c.n; i++)
            data[i] = 0;

        for (int i = 0; i < c.n; i++)
            for (int j = 0; j < c.n; j++)
                if (c.W[i][j] > 0)
                    data[i * c.n + j] = ipamir_val_lit(solver, i * c.n + j) > 0 ? 0 : c.W[i][j];

        int added_lb = 0, old_C = C;
        lazy_cycle_packing(solver, D, c.n, visited, on_stack, prev, &C, &added_lb);

        fprintf(stderr, "%d %d %d\n", old_C, C, added_lb);
        if (old_C == C)
            break;

        fprintf(stderr, "Solving %d hard\n", C);
        ipamir_solve(solver);
        fprintf(stderr, "Solved %d vs %d\n", *c.c, (int)ipamir_val_obj(solver));
    }

    if (*c.c != (int)ipamir_val_obj(solver))
        printf("TODO, fix output if UB not optimal\n");

    ipamir_release(solver);
    free(D);
    free(data);
    return 1;
}