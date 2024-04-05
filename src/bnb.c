#include "bnb.h"
#include "ocm.h"
#include "dfas.h"
#include "heuristics.h"

#include <stdlib.h>

#define MAX_D 200

int bc = 0;
int lr[400];

void bb(ocm *p, ocm *ub, int d)
{
    if (d >= MAX_D - 1)
    {
        exit(1);
    }

    if (p[d].crossings + p[d].lb >= ub->crossings + ub->lb)
        return;

    // bc++;
    // if (bc > 100)
    // {
    //     bc = 0;
    //     printf("\r%d %d %d  ", p[d].lb + p[d].crossings, ub->crossings, d);
    //     fflush(stdout);
    // }

    // ocm_reduction_k_quick(&p[d], ub->crossings + ub->lb);

    int u = -1, v = -1;
    int best = -1;
    for (int i = 0; i < p[d].N; i++)
    {
        for (int j = i + 1; j < p[d].N; j++)
        {
            if (p[d].tc[i][j] || p[d].tc[j][i] || p[d].cm[i][j] == p[d].cm[j][i])
                continue;

            int _u = i, _v = j;
            if (p[d].cm[i][j] > p[d].cm[j][i])
                _u = j, _v = i;

            if (p[d].cm[_u][_v] > best)
            {
                best = p[d].cm[_u][_v];
                u = _u;
                v = _v;
            }
        }
    }

    if (u < 0 && v < 0)
    {
        if (p[d].crossings + p[d].lb < ub->crossings + ub->lb)
            ocm_copy_tc(p[d], ub);

        return;
    }

    // u < v branch
    ocm_copy_tc(p[d], &p[d + 1]);
    ocm_add_edge(&p[d + 1], u, v);
    bb(p, ub, d + 1);

    if (d < 400)
        lr[d] = 1;

    // v < u branch
    ocm_copy_tc(p[d], &p[d + 1]);
    ocm_add_edge(&p[d + 1], v, u);
    bb(p, ub, d + 1);
}

int *solve_cc(graph g)
{
    ocm *p = malloc(sizeof(ocm) * MAX_D);
    p[0] = init_ocm_problem(g);
    for (int i = 1; i < MAX_D; i++)
        p[i] = copy_ocm_problem(p[0]);

    ocm sol = init_ocm_problem(g);
    ocm_simulated_annealing(&sol);

    ocm_reduction_trivial(&p[0]);
    ocm_reduction_twins(g, &p[0]);
    ocm_reduction_2_1(g, &p[0]);
    ocm_reduction_k_full(&p[0], sol.crossings + sol.lb);
    ocm_reduction_independent(&p[0]);

    dfas dp = dfas_construct_instance(p[0]);
    dfas_reduction_cc(dp);
    for (int u = 0; u < dp.N; u++)
    {
        for (int i = dp.V[u]; i < dp.V[u + 1]; i++)
        {
            int v = dp.E[i];
            if (dp.a[i] || p[0].tc[u][v] || p[0].tc[v][u])
                continue;
            ocm_add_edge(&p[0], u, v);
        }
    }
    dfas_free(dp);

    // printf("solving %d instance (%d, %d, %d)\n", g.B, p[0].crossings + p[0].lb, sol.crossings + sol.lb, p[0].undecided);

    bc = 1000;
    bb(p, &sol, 0);

    // printf("\nsolved %d instance (%d)\n", g.B, sol.crossings + sol.lb);

    // Give order to equal pairs
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            if (!sol.tc[i][j] && !sol.tc[j][i])
                ocm_add_edge(&sol, i, j);

    for (int i = 0; i < MAX_D; i++)
        free_ocm_problem(p[i]);

    int *s = malloc(sizeof(int) * g.B);
    for (int i = 0; i < g.B; i++)
        s[i] = -1;

    for (int i = 0; i < g.B; i++)
    {
        int d = 0;
        for (int j = 0; j < g.B; j++)
            if (sol.tc[i][j])
                d++;
        s[g.B - d - 1] = i + g.A;
    }

    free_ocm_problem(sol);

    return s;
}

int *solve_cc_sat(graph g)
{
    ocm p = init_ocm_problem(g);

    ocm_reduction_trivial(&p);
    ocm_reduction_twins(g, &p);
    ocm_reduction_2_1(g, &p);

    dfas dp = dfas_construct_instance(p);
    dfas_reduction_cc(dp);
    for (int u = 0; u < dp.N; u++)
    {
        for (int i = dp.V[u]; i < dp.V[u + 1]; i++)
        {
            int v = dp.E[i];
            if (dp.a[i] || p.tc[u][v] || p.tc[v][u])
                continue;
            ocm_add_edge_avx(&p, u, v);
        }
    }
    dfas_solve_cc(dp);
    for (int u = 0; u < dp.N; u++)
    {
        for (int i = dp.V[u]; i < dp.V[u + 1]; i++)
        {
            int v = dp.E[i];
            if (p.tc[u][v] || p.tc[v][u])
                continue;
            if (dp.a[i])
                ocm_add_edge_avx(&p, u, v);
            else
                ocm_add_edge_avx(&p, v, u);
        }
    }
    dfas_free(dp);

    // Give order to equal pairs
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            if (!p.tc[i][j] && !p.tc[j][i])
                ocm_add_edge_avx(&p, i, j);

    int *s = malloc(sizeof(int) * g.B);
    for (int i = 0; i < g.B; i++)
        s[i] = -1;

    for (int i = 0; i < g.B; i++)
    {
        int d = 0;
        for (int j = 0; j < g.B; j++)
            if (p.tc[i][j])
                d++;
        s[g.B - d] = i + g.A;
    }

    free_ocm_problem(p);

    return s;
}

int *solve_bnb(graph g)
{
    graph gz = remove_degree_zero(g);
    graph gt = remove_twins(gz);
    int N = 0;
    graph *cc = split_graph(gt, &N);

    int *s = malloc(sizeof(int) * gz.B);
    int p = 0;
    for (int i = 0; i < N; i++)
    {
        if (cc[i].B > 1)
        {
            int *s_cc = solve_cc_sat(cc[i]);
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
    free(cc);

    return s;
}
