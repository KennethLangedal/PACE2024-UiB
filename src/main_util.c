#include "graph.h"
#include "ocm.h"
#include "dfas.h"
#include "heuristics.h"
#include "tc.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    int v = validate_graph(g);
    if (v != 1)
        printf("Error in graph %d\n", v);

    graph gz = remove_degree_zero(g);
    graph gt = remove_twins(gz);

    int N = 0;
    graph *sg = split_graph(gt, &N);

    int max = 0, max_cc = 0;
    for (int i = 0; i < N; i++)
    {
        if (sg[i].B < 2)
            continue;

        ocm p = init_ocm_problem(sg[i]);
        ocm_reduction_trivial(&p);
        // ocm_reduction_2_1(gz, &p);

        // int *s = malloc(sizeof(int) * p.N);
        // int ub = simulated_annealing(sg[i], s);
        // free(s);

        // ocm_reduction_k_full(&p, ub);
        // ocm_reduction_independent(&p);

        dfas gp = dfas_construct_instance(p);
        dfas_reduction_cc(gp);
        dfas_solve_cc(gp);

        // return 0;

        for (int u = 0; u < gp.N; u++)
        {
            for (int i = gp.V[u]; i < gp.V[u + 1]; i++)
            {
                int v = gp.E[i];
                if (!gp.a[i] && !p.tc[u][v] && !p.tc[v][u])
                    ocm_add_edge(&p, v, u);
            }
        }
        for (int u = 0; u < p.N; u++)
        {
            for (int v = u + 1; v < p.N; v++)
            {
                if (p.tc[u][v] || p.tc[v][u])
                    continue;
                if (p.cm[u][v] < p.cm[v][u])
                    ocm_add_edge(&p, u, v);
                else
                    ocm_add_edge(&p, v, u);
            }
        }
        // ocm_update(&p);

        for (int i = 0; i < *gp.n_cc; i++)
            if (max_cc < gp.cc_size[i])
                max_cc = gp.cc_size[i];

        if (max < p.undecided)
            max = p.undecided;

        free_ocm_problem(p);
    }

    printf("%d %d\n", max, max_cc);

    // dfas gp = dfas_construct_instance(p);
    // dfas_reduction_cc(gp);
    // f = fopen(argv[2], "w");
    // dfas_store_dfvs(f, gp);
    // fclose(f);

    // int N = 0;
    // for (int u = 0; u < gp.N; u++)
    // {
    //     for (int i = gp.V[u]; i < gp.V[u + 1]; i++)
    //     {
    //         if (gp.a[i])
    //             N++;
    //     }
    // }

    // int M = 0;
    // for (int i = 0; i < *gp.n_cc; i++)
    //     if (M < gp.cc_size[i])
    //         M = gp.cc_size[i];

    // printf("%d %d %d\n", gp.N, M, N);

    // printf("source;target;weight\n");
    // for (int u = 0; u < gp.N; u++)
    // {
    //     for (int i = gp.V[u]; i < gp.V[u + 1]; i++)
    //     {
    //         if (!gp.a[i])
    //             continue;
    //         int v = gp.E[i];
    //         printf("%d;%d;%d\n", u, v, gp.W[i]);
    //     }
    // }

    return 0;
}