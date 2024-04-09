#include "graph.h"
#include "ocm.h"
#include "dfas.h"
#include "tc.h"
#include "heuristics.h"

#include <stdlib.h>
#include <stdio.h>

// int search(ocm *g, dfas p)
// {
//     printf("%d\n", p.V[p.N]);
//     int *mask = malloc(sizeof(int) * p.N);

//     for (int u = 0; u < p.N; u++)
//     {
//         for (int i = 0; i < p.N; i++)
//             mask[i] = 0;

//         mask[u] = 1;
//         int count = 1;
//         for (int i = p.V[u]; i < p.V[u + 1] && count < 30; i++)
//         {
//             int v = p.E[i];
//             if (!mask[v])
//                 count++;
//             mask[v] = 1;
//             for (int j = p.V[v]; j < p.V[v + 1] && count < 30; j++)
//             {
//                 if (mask[p.E[j]])
//                     continue;
//                 mask[p.E[j]] = 1;
//                 count++;
//             }
//         }

//         dfas sub = dfas_construct_subgraph(p, mask);
//         int c1 = 0, c2 = 0;
//         int *s1 = dfas_solve(sub, &c1);

//         int *s2 = dfas_solve_closed(p, mask, c1, &c2);

//         if (c1 == c2)
//         {
//             printf("\rWoho! Reduction by %d\n", count);
//             for (int v = 0; v < p.N; v++)
//             {
//                 for (int i = p.V[v]; i < p.V[v + 1]; i++)
//                 {
//                     if (!mask[p.E[i]])
//                         continue;
//                     if (s2[i])
//                         ocm_add_edge(g, p.id_V[p.E[i]], p.id_V[v]);
//                     else
//                         ocm_add_edge(g, p.id_V[v], p.id_V[p.E[i]]);
//                 }
//             }

//             free(s1);
//             free(s2);
//             free(mask);
//             return 1;
//         }
//         else
//         {
//             printf("\r%d %d     ", c1, c2);
//             fflush(stdout);
//         }

//         free(s1);
//         free(s2);
//     }

//     free(mask);
//     return 0;
// }

void eval_edge(dfas p, int u, int v, int e, int *w_in, int *w_out)
{
    *w_in = p.W[e];

    *w_out = 0;
    for (int i = p.V[v]; i < p.V[v + 1]; i++)
    {
        int w = p.E[i];
        for (int j = p.V[w]; j < p.V[w + 1]; j++)
        {
            if (p.E[j] == u)
                *w_out += p.W[i] < p.W[j] ? p.W[i] : p.W[j];
        }
    }
}

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
    ocm_reduction_trivial(&p);

    int any = 1;
    while (any)
    {
        any = 0;
        dfas dp = dfas_construct_instance(p);

        printf("Undecided pairs %d\n", p.undecided);

        int N = 0;
        dfas *cc = dfas_scc_split(dp, &N);

        for (int i = 0; i < N; i++)
        {
            if (cc[i].N < 100)
                continue;

            int *s = malloc(sizeof(int) * cc[i].N);
            printf("%d\n", dfas_simulated_annealing(cc[i], s));

            // any |= search(&p, cc[i]);
            // int w_in, w_out;
            // for (int u = 0; u < cc[i].N; u++)
            // {
            //     int in = 0, out = 0;
            //     for (int j = cc[i].V[u]; j < cc[i].V[u + 1]; j++)
            //     {
            //         int v = cc[i].E[j];
            //         eval_edge(cc[i], u, v, j, &w_in, &w_out);
            //         if (w_out > out)
            //         {
            //             in = w_in;
            //             out = w_out;
            //         }
            //     }
            //     printf("%d %d %d\n", u, in, out);
            // }
        }
        for (int i = 0; i < N; i++)
            dfas_free(cc[i]);

        free(cc);
        dfas_free(dp);
    }

    free_ocm_problem(p);
    free_graph(gt);
    free_graph(gz);
    free_graph(g);

    // int removed = 0;
    // removed += dfas_reduction_degree_one(dp);
    // printf("%d %d\n", p.undecided, p.undecided - removed);
    // removed += dfas_reduction_degree_one(dp);
    // printf("%d %d\n", p.undecided, p.undecided - removed);
    // removed += dfas_reduction_degree_one(dp);
    // printf("%d %d\n", p.undecided, p.undecided - removed);
    // removed += dfas_reduction_degree_one(dp);
    // printf("%d %d\n", p.undecided, p.undecided - removed);
    // removed += dfas_reduction_degree_one(dp);
    // printf("%d %d\n", p.undecided, p.undecided - removed);

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