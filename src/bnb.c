#include "bnb.h"
#include "ocm.h"
#include "heuristics.h"

#include <stdlib.h>

int bc = 0;
int lr[400];

int lock_in_k(int **cm, int **tc, int N, int crossings, int ub)
{
    int lb = lower_bound(cm, tc, N);
    int found = 1;
    while (found)
    {
        found = 0;
        for (int i = 0; i < N; i++)
        {
            for (int j = i + 1; j < N; j++)
            {
                if (!tc[i][j] && !tc[j][i])
                {
                    if (cm[i][j] < cm[j][i] && crossings + (lb - cm[i][j]) + cm[j][i] >= ub)
                    {
                        crossings += tc_add_edge(tc, tc, N, i, j, cm);
                        lb = lower_bound(cm, tc, N);
                        found = 1;
                    }
                    else if (cm[j][i] < cm[i][j] && crossings + (lb - cm[j][i]) + cm[i][j] >= ub)
                    {
                        crossings += tc_add_edge(tc, tc, N, j, i, cm);
                        lb = lower_bound(cm, tc, N);
                        found = 1;
                    }
                }
            }
        }
    }
    return crossings;
}

void bb(int **cm, int **tc, int N, int crossings, int **sol, int *ub, int d)
{
    if (crossings + lower_bound(cm, tc, N) >= *ub)
        return;

    // crossings = lock_in_k(cm, tc, N, crossings, *ub);

    // find good pair
    int u = -1, v = -1;
    int best = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            if (tc[i][j] == 0 && tc[j][i] == 0 && cm[i][j] != cm[j][i])
            {
                int cost_p = tc_try_add_edge(tc, N, i, j, cm);
                int cost_n = tc_try_add_edge(tc, N, j, i, cm);
                int cost = cost_p < cost_n ? cost_p : cost_n;
                if (u < 0 || cost > best)
                {
                    if (cost_p < cost_n)
                        u = i, v = j;
                    else
                        u = j, v = i;
                    best = cost;
                }
            }
        }
    }

    if (u < 0 && v < 0)
    {
        int unset = 0;
        for (int i = 0; i < N; i++)
            for (int j = i + 1; j < N; j++)
                if (tc[i][j] == tc[j][i])
                    unset += cm[i][j];

        if (crossings + unset < *ub)
        {
            *ub = crossings + unset;
            for (int i = 0; i < N * N; i++)
                (*sol)[i] = (*tc)[i];

            int sanity_check = 0;
            for (int i = 0; i < N; i++)
            {
                for (int j = i + 1; j < N; j++)
                {
                    if (cm[i][j] != cm[j][i] && sol[i][j] == sol[j][i])
                        printf("Error\n");
                    if (sol[i][j])
                        sanity_check += cm[i][j];
                    else
                        sanity_check += cm[j][i];
                }
            }
            // printf("%d %d %d\n", crossings + unset, sanity_check, d);
        }
        return;
    }

    bc++;
    if (d < 400)
        lr[d] = 0;
    if (!(bc % 100))
    {
        bc = 0;
        printf("\r");
        printf("%d ", *ub);
        for (int i = 0; i < 100; i++)
            printf("%d", lr[i]);
        fflush(stdout);
    }

    int **_tc = init_tc(N);

    // u < v branch
    int b1 = tc_add_edge(tc, _tc, N, u, v, cm) + crossings;
    bb(cm, _tc, N, b1, sol, ub, d + 1);

    if (d < 400)
        lr[d] = 1;

    // v < u branch
    int b2 = tc_add_edge(tc, _tc, N, v, u, cm) + crossings;
    bb(cm, _tc, N, b2, sol, ub, d + 1);

    free_tc(_tc);
}

int *solve_cc(graph g)
{
    if (g.B == 0)
        return NULL;

    if (g.B == 1)
    {
        int *s = malloc(sizeof(int));
        s[0] = g.A;
        return s;
    }

    int **cm = init_cost_matrix(g);
    int **tc = init_tc(g.B);
    int **sol = init_tc(g.B);

    tc_add_trivial(tc, cm, g.B);
    tc_add_independent(tc, cm, g.B);

    tc_add_trivial(sol, cm, g.B);
    tc_add_independent(tc, cm, g.B);
    int cost = upper_bound(cm, sol, g.B);

    int heur = greedy_placement(g.B, cm);

    printf("solving %d instance (%d)(%d)\n", g.B, cost, heur);
    // FILE *f = fopen("p.gr", "w");
    // store_graph(f, g);
    // fclose(f);

    cost = heur + 1;
    bb(cm, tc, g.B, 0, sol, &cost, 0);

    printf("\nsolved %d instance (%d)\n", g.B, cost);

    // Give order to equal pairs
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            if (!sol[i][j] && !sol[j][i])
                tc_add_edge(sol, sol, g.B, i, j, cm);

    free_cost_matrix(cm);
    free_tc(tc);

    int *s = malloc(sizeof(int) * g.B);
    for (int i = 0; i < g.B; i++)
    {
        int d = 0;
        for (int j = 0; j < g.B; j++)
            if (sol[i][j])
                d++;
        s[g.B - d - 1] = i + g.A;
    }

    free_tc(sol);

    return s;
}

int *solve_post_degree_one(graph g)
{
    int N = 0;
    graph *cc = split_graph(g, &N);

    int *s = malloc(sizeof(int) * g.B);
    int p = 0;

    for (int i = 0; i < N; i++)
    {
        int *_s = solve_cc(cc[i]);
        for (int j = 0; j < cc[i].B; j++)
            s[p++] = cc[i].old_label[_s[j]];
        free(_s);
        free_graph(cc[i]);
    }
    free(cc);

    return s;
}

int *find_degree_one(graph g, int nd)
{
    int *d = malloc(sizeof(int) * nd);

    int p = 0;
    for (int u = 0; u < g.A; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            if (g.V[v + 1] - g.V[v] == 1)
                d[p++] = v;
        }
    }

    for (int u = g.A; u < g.N; u++)
    {
        if (g.V[u + 1] - g.V[u] == 0)
            d[p++] = u;
    }

    return d;
}

int *solve_post_twin_removal(graph g)
{
    graph _g = remove_degree_one(g);

    int *_s = solve_post_degree_one(_g);
    for (int i = 0; i < _g.B; i++)
        _s[i] = _g.old_label[_s[i]];

    if (g.B == _g.B)
    {
        free_graph(_g);
        return _s;
    }

    int *_d = find_degree_one(g, g.B - _g.B);

    int *s = malloc(sizeof(int) * g.B);

    // Merge

    int p = 0, p1 = 0, p2 = 0;
    while (p < g.B && p1 < _g.B && p2 < g.B - _g.B)
    {
        int i = p;
        int cost = 0;
        for (int j = p1; j < _g.B; j++)
            cost += number_of_crossings(g, _d[p2], _s[j]);

        int best = p1;
        int best_cost = cost;
        for (int j = p1; j < _g.B; j++)
        {
            cost -= number_of_crossings(g, _d[p2], _s[j]);
            cost += number_of_crossings(g, _s[j], _d[p2]);
            if (cost < best_cost)
            {
                best = j + 1;
                best_cost = cost;
            }
        }

        for (int j = p1; j < best; j++)
            s[p++] = _s[j];
        p1 += best - p1;

        s[p++] = _d[p2++];
    }

    while (p1 < _g.B)
        s[p++] = _s[p1++];

    while (p2 < g.B - _g.B)
        s[p++] = _d[p2++];

    free_graph(_g);
    free(_s);
    free(_d);

    return s;
}

void add_twins(graph g, int u, int *s, int *p)
{
    for (int v = g.A; v < g.N; v++)
    {
        if (v != u && test_twin(g, u, v))
            s[(*p)++] = v;
    }
}

int *solve_bnb(graph g)
{
    graph _g = remove_twins(g);

    int *_s = solve_post_twin_removal(_g);

    int *s = malloc(sizeof(int) * g.B);

    int p = 0;
    for (int i = 0; i < _g.B; i++)
    {
        int u = _s[i];
        s[p++] = _g.old_label[u];
        if (_g.twins[u] > 0)
            add_twins(g, _g.old_label[u], s, &p);
    }

    free_graph(_g);
    free(_s);
    return s;
}

int lower_bound(int **cm, int **tc, int N)
{
    int lb = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            if (!tc[i][j] && !tc[j][i])
                lb += cm[i][j] < cm[j][i] ? cm[i][j] : cm[j][i];
        }
    }
    return lb;
}

int upper_bound(int **cm, int **tc, int N)
{
    int crossings = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            if (!tc[i][j] && !tc[j][i])
            {
                int cost_ij = tc_try_add_edge(tc, N, i, j, cm);
                int cost_ji = tc_try_add_edge(tc, N, j, i, cm);
                if (cost_ij < cost_ji)
                    crossings += tc_add_edge(tc, tc, N, i, j, cm);
                else
                    crossings += tc_add_edge(tc, tc, N, j, i, cm);
            }
        }
    }
    return crossings;
}