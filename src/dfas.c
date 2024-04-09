#include "dfas.h"
#include "ipamir.h"
#include "heuristics.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static inline int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

dfas dfas_construct_instance(ocm p)
{
    dfas g = {.N = p.N};
    g.V = malloc(sizeof(int) * (p.N + 1));
    g.E = malloc(sizeof(int) * p.undecided);
    g.W = malloc(sizeof(int) * p.undecided);

    g.id_V = malloc(sizeof(int) * p.N);
    g.id_E = malloc(sizeof(int) * p.undecided);

    for (int i = 0; i <= p.N; i++)
        g.V[i] = 0;

    for (int i = 0; i < p.N; i++)
        g.id_V[i] = i;

    for (int i = 0; i < p.undecided; i++)
        g.id_E[i] = i;

    for (int i = 0; i < p.N; i++)
    {
        for (int j = i + 1; j < p.N; j++)
        {
            if (p.tc[i][j] || p.tc[j][i] || p.cm[i][j] == p.cm[j][i])
                continue;

            if (p.cm[i][j] < p.cm[j][i])
                g.V[i]++;
            else
                g.V[j]++;
        }
    }

    for (int i = 1; i <= p.N; i++)
        g.V[i] += g.V[i - 1];

    for (int i = 0; i < p.N; i++)
    {
        for (int j = i + 1; j < p.N; j++)
        {
            if (p.tc[i][j] || p.tc[j][i] || p.cm[i][j] == p.cm[j][i])
                continue;

            if (p.cm[i][j] < p.cm[j][i])
            {
                g.V[i]--;
                g.E[g.V[i]] = j;
            }
            else
            {
                g.V[j]--;
                g.E[g.V[j]] = i;
            }
        }
    }

    for (int i = 0; i < g.N; i++)
        qsort(g.E + g.V[i], g.V[i + 1] - g.V[i], sizeof(int), compare);

    for (int u = 0; u < g.N; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            g.W[i] = p.cm[v][u] - p.cm[u][v];
        }
    }

    return g;
}

dfas dfas_construct_subgraph(dfas p, int *mask)
{
    int *new_id = malloc(sizeof(int) * p.N);
    for (int i = 0; i < p.N; i++)
        new_id[i] = -1;

    int id = 0;
    for (int u = 0; u < p.N; u++)
        if (mask[u])
            new_id[u] = id++;

    dfas s = {.N = id};
    s.V = malloc(sizeof(int) * (s.N + 1));
    s.E = malloc(sizeof(int) * p.V[p.N]);
    s.W = malloc(sizeof(int) * p.V[p.N]);

    s.id_V = malloc(sizeof(int) * s.N);
    s.id_E = malloc(sizeof(int) * p.V[p.N]);

    id = 0;
    int id_e = 0;
    for (int u = 0; u < p.N; u++)
    {
        if (!mask[u])
            continue;

        int _u = id++;

        s.V[_u] = id_e;
        s.id_V[_u] = u;

        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            int v = p.E[i];
            if (!mask[v])
                continue;

            int _v = new_id[v];
            s.E[id_e] = _v;
            s.W[id_e] = p.W[i];
            s.id_E[id_e] = i;
            id_e++;
        }
    }

    s.V[s.N] = id_e;
    s.E = realloc(s.E, sizeof(int) * id_e);
    s.W = realloc(s.W, sizeof(int) * id_e);
    s.id_E = realloc(s.id_E, sizeof(int) * id_e);

    free(new_id);
    return s;
}

void dfas_free(dfas p)
{
    free(p.V);
    free(p.E);
    free(p.W);
    free(p.id_V);
    free(p.id_E);
}

typedef struct
{
    int N;
    int *stack, *on_stack;
    int Id;
    int *index, *lowlink;
} tarjan;

tarjan init_tarjan(int N)
{
    tarjan t = {.N = 0, .Id = 0};
    t.stack = malloc(sizeof(int) * N);
    t.on_stack = malloc(sizeof(int) * N);
    t.index = malloc(sizeof(int) * N);
    t.lowlink = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++)
    {
        t.on_stack[i] = 0;
        t.index[i] = -1;
    }
    return t;
}

void free_tarjan(tarjan t)
{
    free(t.stack);
    free(t.on_stack);
    free(t.index);
    free(t.lowlink);
}

void tarjan_explore(int u, dfas p, tarjan *t, dfas *cc, int *ncc, int *mask)
{
    t->index[u] = t->Id;
    t->lowlink[u] = t->Id;
    t->Id += 1;

    t->stack[t->N] = u;
    t->N += 1;
    t->on_stack[u] = 1;

    for (int i = p.V[u]; i < p.V[u + 1]; i++)
    {
        int v = p.E[i];
        if (t->index[v] < 0)
        {
            tarjan_explore(v, p, t, cc, ncc, mask);
            if (t->lowlink[v] < t->lowlink[u])
                t->lowlink[u] = t->lowlink[v];
        }
        else if (t->on_stack[v] && t->index[v] < t->lowlink[u])
        {
            t->lowlink[u] = t->index[v];
        }
    }

    if (t->lowlink[u] == t->index[u])
    {
        for (int i = 0; i < p.N; i++)
            mask[i] = 0;

        int v;
        do
        {
            t->N -= 1;
            v = t->stack[t->N];
            t->on_stack[v] = 0;
            mask[v] = 1;
        } while (v != u);

        cc[(*ncc)++] = dfas_construct_subgraph(p, mask);
    }
}

dfas *dfas_scc_split(dfas p, int *N)
{
    dfas *cc = malloc(sizeof(dfas) * p.N);
    *N = 0;
    tarjan t = init_tarjan(p.N);

    int *mask = malloc(sizeof(int) * p.N);
    for (int u = 0; u < p.N; u++)
    {
        if (t.index[u] < 0)
            tarjan_explore(u, p, &t, cc, N, mask);
    }

    free(mask);
    free_tarjan(t);
    cc = realloc(cc, sizeof(dfas) * (*N));
    return cc;
}

void push_cycle(void *solver, int N, int *cycle)
{
    for (int i = 0; i < N; i++)
        ipamir_add_hard(solver, cycle[i] + 1);
    ipamir_add_hard(solver, 0);
}

int cycle_explore(dfas g, void *solver, int *sat, int d, int *e, int *v, int *visited, int *on_stack, int max_l, int *any)
{
    int u = v[d];
    int res = 0;
    for (int i = g.V[u]; i < g.V[u + 1]; i++)
    {
        if (sat[i])
            continue;

        e[d] = i;
        int w = g.E[i];
        if (on_stack[w]) // cycle
        {
            int offset = 0;
            while (v[offset] != w)
                offset++;
            int size = (d - offset) + 1;
            if (size <= max_l)
            {
                push_cycle(solver, size, e + offset);
                res++;
            }
            *any = 1;
        }
        else if (!visited[w])
        {
            visited[w] = 1;
            on_stack[w] = 1;
            v[d + 1] = w;
            res += cycle_explore(g, solver, sat, d + 1, e, v, visited, on_stack, max_l, any);
            on_stack[w] = 0;
        }
    }

    return res;
}

void shuffle(int *array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int add_cycles(dfas g, void *solver, int *sat, int max_l, int *any)
{
    int *visited = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        visited[i] = 0;

    int *on_stack = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        on_stack[i] = 0;

    int *order = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        order[i] = i;
    shuffle(order, g.N);

    int *e = malloc(sizeof(int) * g.N);
    int *v = malloc(sizeof(int) * g.N);

    int n_cycles = 0;
    for (int i = 0; i < g.N; i++)
    {
        int u = order[i];
        if (visited[u])
            continue;

        v[0] = u;
        visited[u] = 1;
        on_stack[u] = 1;
        n_cycles += cycle_explore(g, solver, sat, 0, e, v, visited, on_stack, max_l, any);
        on_stack[u] = 0;
    }
    free(visited);
    free(on_stack);
    free(e);
    free(v);
    return n_cycles;
}

int add_4_cycles(dfas g, void *solver)
{
    int res = 0;
    int cycle[4];
    for (int u = 0; u < g.N; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            if (v < u)
                continue;

            for (int j = g.V[v]; j < g.V[v + 1]; j++)
            {
                int w = g.E[j];
                if (w < u)
                    continue;

                for (int k = g.V[w]; k < g.V[w + 1]; k++)
                {
                    int x = g.E[k];
                    if (x < u)
                        continue;

                    if (x == u) // && drand48() > 0.9)
                    {
                        cycle[0] = i;
                        cycle[1] = j;
                        cycle[2] = k;
                        push_cycle(solver, 3, cycle);
                        res++;
                    }
                    else if (x != u)
                    {
                        int adj_v = 0, adj_u = 0;
                        for (int l = g.V[x]; l < g.V[x + 1]; l++)
                        {
                            if (g.E[l] == u)
                            {
                                cycle[3] = l;
                                adj_u = 1;
                            }
                            else if (g.E[l] == v)
                                adj_v = 1;
                        }
                        if (adj_u && !adj_v) // && drand48() > 0.999)
                        {
                            cycle[0] = i;
                            cycle[1] = j;
                            cycle[2] = k;
                            push_cycle(solver, 4, cycle);
                            res++;
                        }
                    }
                }
            }
        }
    }
    return res;
}

int *dfas_solve(dfas g, int *c)
{
    int *sat = malloc(sizeof(int) * g.V[g.N]);
    for (int i = 0; i < g.V[g.N]; i++)
        sat[i] = 0;

    void *solver = ipamir_init();
    for (int i = 0; i < g.V[g.N]; i++)
        ipamir_add_soft_lit(solver, i + 1, g.W[i]);

    int K = add_4_cycles(g, solver);
    int any = K > 0, max_l = 4;
    int C = K;

    // printf("Starting problem of size %d\n", g.V[g.N]);

    while (any)
    {
        // printf("%d %d %d", K, C, max_l);
        // fflush(stdout);
        int rc = ipamir_solve(solver);
        // printf(" %d %ld\n", rc, ipamir_val_obj(solver));

        for (int i = 0; i < g.V[g.N]; i++)
            sat[i] = ipamir_val_lit(solver, i + 1) > 0;

        *c = ipamir_val_obj(solver);

        any = 0;
        K = add_cycles(g, solver, sat, max_l, &any);
        C += K;
        while (K == 0 && any)
        {
            max_l++;
            any = 0;
            K = add_cycles(g, solver, sat, max_l, &any);
            C += K;
        }
    }

    ipamir_release(solver);

    return sat;
}

// int cycle_explore_masked(dfas g, void *solver, int *sat, int d, int *e, int *v, int *visited, int *on_stack, int max_l, int *any, int *mask)
// {
//     int u = v[d];
//     int res = 0;
//     for (int i = g.V[u]; i < g.V[u + 1]; i++)
//     {
//         if (sat[i])
//             continue;

//         e[d] = i;
//         int w = g.E[i];
//         v[d + 1] = w;
//         if (on_stack[w]) // cycle
//         {
//             int offset = 0;
//             while (v[offset] != w)
//                 offset++;
//             int size = (d - offset) + 1;

//             int valid = 0;
//             for (int j = 0; j < size; j++)
//                 if (mask[v[offset + j]] && mask[v[offset + j + 1]])
//                     valid = 1;
//             if (!valid)
//                 continue;

//             if (size <= max_l)
//             {
//                 for (int j = 0; j < size; j++)
//                 {
//                     if (mask[v[offset + j]] && mask[v[offset + j + 1]])
//                     {
//                         ipamir_add_hard(solver, e[offset + j] + 1);
//                     }
//                 }
//                 ipamir_add_hard(solver, 0);
//                 res++;
//             }
//             *any = 1;
//         }
//         else if (!visited[w])
//         {
//             visited[w] = 1;
//             on_stack[w] = 1;
//             res += cycle_explore_masked(g, solver, sat, d + 1, e, v, visited, on_stack, max_l, any, mask);
//             on_stack[w] = 0;
//         }
//     }

//     return res;
// }

// int add_cycles_masked(dfas g, void *solver, int *sat, int max_l, int *any, int *mask)
// {
//     int *visited = malloc(sizeof(int) * g.N);
//     for (int i = 0; i < g.N; i++)
//         visited[i] = 0;

//     int *on_stack = malloc(sizeof(int) * g.N);
//     for (int i = 0; i < g.N; i++)
//         on_stack[i] = 0;

//     int *order = malloc(sizeof(int) * g.N);
//     for (int i = 0; i < g.N; i++)
//         order[i] = i;
//     shuffle(order, g.N);

//     int *e = malloc(sizeof(int) * g.N);
//     int *v = malloc(sizeof(int) * g.N);

//     int n_cycles = 0;
//     for (int i = 0; i < g.N; i++)
//     {
//         int u = order[i];
//         if (visited[u])
//             continue;

//         v[0] = u;
//         visited[u] = 1;
//         on_stack[u] = 1;
//         n_cycles += cycle_explore_masked(g, solver, sat, 0, e, v, visited, on_stack, max_l, any, mask);
//         on_stack[u] = 0;
//     }
//     free(visited);
//     free(on_stack);
//     free(e);
//     free(v);
//     return n_cycles;
// }

// int *dfas_solve_closed(dfas g, int *mask, int c1, int *c2)
// {
//     int *sat = malloc(sizeof(int) * g.V[g.N]);
//     for (int i = 0; i < g.V[g.N]; i++)
//         sat[i] = 0;

//     void *solver = ipamir_init();
//     for (int u = 0; u < g.N; u++)
//         for (int i = g.V[u]; i < g.V[u + 1]; i++)
//             if (mask[u] && mask[g.E[i]])
//                 ipamir_add_soft_lit(solver, i + 1, g.W[i]);

//     int any = 0, max_l = 4;
//     int K = add_cycles_masked(g, solver, sat, max_l, &any, mask);
//     int C = K;

//     // printf("Starting problem of size %d\n", g.V[g.N]);

//     while (any)
//     {
//         // printf("%d %d %d", K, C, max_l);
//         // fflush(stdout);
//         int rc = ipamir_solve(solver);
//         // printf(" %d %ld\n", rc, ipamir_val_obj(solver));

//         for (int u = 0; u < g.N; u++)
//             for (int i = g.V[u]; i < g.V[u + 1]; i++)
//                 if (mask[u] && mask[g.E[i]])
//                     sat[i] = ipamir_val_lit(solver, i + 1) > 0;

//         *c2 = ipamir_val_obj(solver);

//         if (*c2 > c1)
//             break;

//         any = 0;
//         K = add_cycles_masked(g, solver, sat, max_l, &any, mask);
//         C += K;
//         while (K == 0 && any)
//         {
//             max_l++;
//             any = 0;
//             K = add_cycles_masked(g, solver, sat, max_l, &any, mask);
//             C += K;
//         }
//     }

//     ipamir_release(solver);

//     return sat;
// }