#include "bnb.h"
#include "ocm.h"
#include "heuristics.h"

#include <stdlib.h>

int bc = 0;
int lr[400];

void bb(ocm *p, ocm *ub, int d)
{
    if (p[d].crossings + p[d].lb >= ub->crossings + ub->lb)
        return;

    // bc++;
    // if (bc > 100)
    // {
    //     bc = 0;
    //     printf("\r%d %d %d  ", p[d].lb + p[d].crossings, ub->crossings, d);
    //     fflush(stdout);
    // }

    ocm_reduction_k_quick(&p[d], ub->crossings + ub->lb);

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
    ocm *p = malloc(sizeof(ocm) * 1000);
    for (int i = 0; i < 1000; i++)
        p[i] = init_ocm_problem(g);

    ocm sol = init_ocm_problem(g);
    ocm_simulated_annealing(&sol);

    ocm_reduction_trivial(&p[0]);
    ocm_reduction_twins(g, &p[0]);
    ocm_reduction_2_1(g, &p[0]);
    ocm_reduction_independent(&p[0]);
    ocm_reduction_k_full(&p[0], sol.crossings + sol.lb);

    printf("solving %d instance (%d, %d, %d)\n", g.B, p[0].crossings + p[0].lb, sol.crossings + sol.lb, sol.undecided);

    bc = 1000;
    bb(p, &sol, 0);

    printf("\nsolved %d instance (%d)\n", g.B, sol.crossings + sol.lb);

    // Give order to equal pairs
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            if (!sol.tc[i][j] && !sol.tc[j][i])
                ocm_add_edge(&sol, i, j);

    for (int i = 0; i < 1000; i++)
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

int *solve_bnb(graph g)
{
    graph gz = remove_degree_zero(g);
    int N = 0;
    graph *cc = split_graph(gz, &N);

    int *s = malloc(sizeof(int) * gz.B);
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

    lift_solution_degree_zero(g, gz, &s);

    free_graph(gz);
    free(cc);

    return s;
}
