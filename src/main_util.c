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

    ocm p = init_ocm_problem(gt);
    dfas dp = dfas_construct_instance(p);
    dfas_reduction_cc(dp);

    int *degree_in = malloc(sizeof(int) * dp.N);
    int *degree_out = malloc(sizeof(int) * dp.N);

    for (int u = 0; u < dp.N; u++)
    {
        degree_in[u] = 0;
        degree_out[u] = 0;
    }
    for (int u = 0; u < dp.N; u++)
    {
        for (int i = dp.V[u]; i < dp.V[u + 1]; i++)
        {
            int v = dp.E[i];
            if (!dp.a[i])
                continue;
            degree_in[v]++;
            degree_out[u]++;
        }
    }
    int total = 0;
    int removeable = 0;
    for (int u = 0; u < dp.N; u++)
    {
        total += degree_out[u];
        if (degree_in[u] == 1)
            removeable += degree_out[u];
        else if (degree_out[u] == 1)
            removeable += degree_in[u];
    }
    printf("%d %d\n", total, removeable);

    // printf("source;target;weight\n");
    // for (int u = 0; u < dp.N; u++)
    // {
    //     for (int i = dp.V[u]; i < dp.V[u + 1]; i++)
    //     {
    //         int v = dp.E[i];
    //         if (!dp.a[i])
    //             continue;
    //         printf("%d;%d;%d\n", u, v, dp.W[i]);
    //     }
    // }

    // int *edges_in_cc = malloc(sizeof(int) * *dp.n_cc);
    // for (int i = 0; i < *dp.n_cc; i++)
    //     edges_in_cc[i] = 0;

    // int max_edges = 0, total_edges = 0, max_nodes = 0;
    // for (int i = 0; i < p.undecided; i++)
    // {
    //     if (dp.a[i])
    //     {
    //         total_edges++;
    //         edges_in_cc[dp.cc[dp.E[i]]]++;
    //     }
    // }

    // for (int c = 0; c < *dp.n_cc; c++)
    // {
    //     if (edges_in_cc[c] > max_edges)
    //         max_edges = edges_in_cc[c];
    //     if (dp.cc_size[c] > max_nodes)
    //         max_nodes = dp.cc_size[c];
    // }

    // printf("%d %d %d %d %d %d\n", g.B, gz.B, gt.B, p.undecided, max_edges, max_nodes);

    return 0;
}