#include "graph.h"
#include "ocm.h"
#include "dfas.h"

#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>

int *solve(graph g)
{
    graph gz = remove_degree_zero(g);
    graph gt = remove_twins(gz);

    ocm p = init_ocm_problem(gt);
    ocm_reduction_trivial(&p);

    dfas dp = dfas_construct_instance(p);

    int N = 0;
    dfas *cc = dfas_scc_split(dp, &N);

    int c;
    for (int i = 0; i < N; i++)
    {
        if (cc[i].N < 3)
            continue;

        int *s = dfas_solve(cc[i], &c);
        for (int u = 0; u < cc[i].N; u++)
        {
            for (int j = cc[i].V[u]; j < cc[i].V[u + 1]; j++)
            {
                int v = cc[i].E[j];
                if (s[j])
                    ocm_add_edge_avx(&p, cc[i].id_V[v], cc[i].id_V[u]);
            }
        }
        free(s);
        dfas_free(cc[i]);
    }
    free(cc);
    dfas_free(dp);

    // Give order to rest
    for (int i = 0; i < gt.B; i++)
        for (int j = 0; j < gt.B; j++)
            if (!p.tc[i][j] && !p.tc[j][i] && p.cm[i][j] < p.cm[j][i])
                ocm_add_edge_avx(&p, i, j);

    // Give order to equal pairs
    for (int i = 0; i < gt.B; i++)
        for (int j = i + 1; j < gt.B; j++)
            if (!p.tc[i][j] && !p.tc[j][i])
                ocm_add_edge_avx(&p, i, j);

    int *s = malloc(sizeof(int) * gt.B);
    for (int i = 0; i < gt.B; i++)
        s[i] = -1;

    for (int i = 0; i < gt.B; i++)
    {
        int d = 0;
        for (int j = 0; j < gt.B; j++)
            if (p.tc[i][j])
                d++;
        s[gt.B - d] = i + gt.A;
    }

    free_ocm_problem(p);

    lift_solution_twins(gz, gt, &s);
    lift_solution_degree_zero(g, gz, &s);

    free_graph(gt);
    free_graph(gz);

    return s;
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    int *s = solve(g);

    // printf("%d\n", count_crossings(g, s));

    f = fopen(argv[2], "w");
    for (int i = 0; i < g.B; i++)
        fprintf(f, "%d\n", s[i] + 1);
    fclose(f);

    free_graph(g);
    free(s);

    return 0;
}