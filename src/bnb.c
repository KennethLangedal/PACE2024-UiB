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
    int lb = lower_bound(cm, tc, N);

    // printf("\r%d %d %d", lb + crossings, *ub, d);
    // fflush(stdout);

    if (crossings + lb >= *ub)
        return;

    // find good pair and lock in forced
    int u = -1, v = -1;
    int best = 0;
    for (int i = 0; i < N; i++)
    {
        for (int j = i + 1; j < N; j++)
        {
            if (!tc[i][j] && !tc[j][i] && cm[i][j] != cm[j][i])
            {
                int f = i, l = j;
                if (cm[j][i] < cm[i][j])
                    f = j, l = i;

                // if (crossings + (lb - cm[f][l]) + cm[l][f] >= *ub) // k-rule
                // {
                //     crossings += tc_add_edge(tc, tc, N, f, l, cm);
                //     lb = lower_bound(cm, tc, N);
                // }
                if (u < 0 || cm[l][f] - cm[f][l] > best)
                {
                    u = l, v = f;
                    best = cm[l][f] - cm[f][l];
                }
            }
        }
    }

    if (crossings + lb >= *ub)
        return;

    if (u < 0 && v < 0)
    {
        if (crossings + lb < *ub)
        {
            *ub = crossings + lb;
            for (int i = 0; i < N * N; i++)
                (*sol)[i] = (*tc)[i];
        }
        return;
    }

    bc++;
    if (d < 400)
        lr[d] = 0;
    // if (!(bc % 100))
    // {
    //     bc = 0;
    //     printf("\r");
    //     printf("%d ", *ub);
    //     for (int i = 0; i < 150; i++)
    //         printf("%d", lr[i]);
    //     fflush(stdout);
    // }

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
    int **cm = init_cost_matrix(g);
    int **tc = init_tc(g.B);

    int **sol = init_tc(g.B);
    int ub = simulated_annealing_cm(cm, sol, g.B);

    tc_add_trivial(tc, cm, g.B);
    int cost = tc_add_independent(tc, cm, g.B);
    cost = lock_in_k(cm, tc, g.B, cost, ub);

    printf("solving %d instance (%d)\n", g.B, ub);

    bb(cm, tc, g.B, cost, sol, &ub, 0);

    printf("\nsolved %d instance (%d)\n", g.B, ub);

    // Give order to equal pairs
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            if (!sol[i][j] && !sol[j][i])
                tc_add_edge(sol, sol, g.B, i, j, cm);

    free_cost_matrix(cm);
    free_tc(tc);

    int *s = malloc(sizeof(int) * g.B);
    for (int i = 0; i < g.B; i++)
        s[i] = -1;

    for (int i = 0; i < g.B; i++)
    {
        int d = 0;
        for (int j = 0; j < g.B; j++)
            if (sol[i][j])
                d++;
        if (s[g.B - d - 1] >= 0)
            printf("Duplicate output\n");
        s[g.B - d - 1] = i + g.A;
    }

    free_tc(sol);

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
    printf("%d\n", g.B);
    graph gz = remove_degree_zero(g);
    printf("%d\n", gz.B);
    graph gt = remove_twins(gz);
    printf("%d\n", gt.B);
    int N = 0;
    graph *cc = split_graph(gt, &N);

    int *s = malloc(sizeof(int) * gt.B);
    int p = 0;
    for (int i = 0; i < N; i++)
    {
        if (cc[i].B > 1)
        {
            int *s_cc = solve_cc(cc[i]);
            for (int j = 0; j < cc[i].B; j++)
                s[p++] = cc[i].old_label[s_cc[j]];

            free(s_cc);
        }
        else if (cc[i].B == 1)
        {
            s[p++] = cc[i].old_label[cc[i].A];
        }
        free_graph(cc[i]);
    }

    lift_solution_twins(gz, gt, &s);
    lift_solution_degree_zero(g, gz, &s);

    free_graph(gz);
    free_graph(gt);
    free(cc);

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

int lower_bound_improved(int **cm, int **tc, int N)
{
    int **set_pairs = malloc(sizeof(int *) * N);
    *set_pairs = malloc(sizeof(int) * N * N);
    for (int i = 0; i < N; i++)
        set_pairs[i] = (*set_pairs) + N * i;

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            set_pairs[i][j] = 0;

    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            set_pairs[i][j] = tc[i][j] || tc[j][i];

    int crossings = 0;
    for (int u = 0; u < N; u++)
    {
        for (int v = u + 1; v < N; v++)
        {
            if (set_pairs[u][v])
                continue;

            int f = u, l = v;
            if (cm[v][u] < cm[u][v])
                f = v, l = u;

            for (int w = v + 1; w < N; w++)
            {
                if (set_pairs[u][w] || set_pairs[v][w])
                    continue;

                if (cm[l][w] < cm[w][l] && cm[w][f] < cm[f][w])
                {
                    // Found cycle
                    set_pairs[u][v] = 1;
                    set_pairs[v][w] = 1;
                    set_pairs[u][w] = 1;

                    int flw = cm[f][l] + cm[l][w] + cm[f][w];
                    int wfl = cm[w][f] + cm[f][l] + cm[w][l];
                    int lwf = cm[l][w] + cm[w][f] + cm[l][f];

                    if (flw < wfl && flw < lwf)
                        crossings += flw;
                    else if (wfl < lwf)
                        crossings += wfl;
                    else
                        crossings += lwf;

                    break;
                }
            }
            if (set_pairs[u][v])
                continue;

            set_pairs[u][v] = 1;
            crossings += cm[f][l];
        }
    }

    free(*set_pairs);
    free(set_pairs);

    return crossings;
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