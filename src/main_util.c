#include "graph.h"
#include "ocm.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    // if (validate_graph(g))
    //     printf("%d %d %d %d\n", g.N, g.V[g.N], g.A, g.B);
    // else
    //     printf("Error in graph\n");

    graph g1 = remove_degree_one(g);
    if (g1.B <= 1)
        return 0;
    // if (validate_graph(g1))
    //     printf("%d %d %d %d\n", g1.N, g1.V[g1.N], g1.A, g1.B);
    // else
    //     printf("Error in graph after degree 1 removal\n");

    graph g2 = remove_twins(g1);
    if (g1.B <= 1)
        return 0;
    // if (validate_graph(g2))
    //     printf("%d %d %d %d\n", g2.N, g2.V[g2.N], g2.A, g2.B);
    // else
    //     printf("Error in graph after twin removal\n");

    int N;
    graph *cc = split_graph(g2, &N);

    int max_B = 0;
    for (int i = 0; i < N; i++)
    {
        if (cc[i].B > cc[max_B].B)
            max_B = i;
    }

    graph lc = cc[max_B];
    if (lc.B <= 1)
        return 0;

    int **cm = init_cost_matrix(lc);
    int **tc = init_tc(lc.B);
    tc_add_trivial(tc, cm, lc.B);
    tc_add_independent(tc, cm, lc.B);

    int decided = 0, undecided = 0;
    for (int i = 0; i < lc.B; i++)
    {
        for (int j = i + 1; j < lc.B; j++)
        {
            if (tc[i][j] || tc[j][i])
                decided++;
            else
                undecided++;
        }
    }

    int removable = 0;
    for (int i = 0; i < lc.B; i++)
    {
        int c = 0;
        for (int j = 0; j < lc.B; j++)
        {
            if (i != j && (tc[i][j] || tc[j][i]))
                c++;
        }
        if (c == lc.B - 1)
            removable++;
    }

    printf("%d %d %.2lf%%\n", lc.B - removable, undecided, (double)decided / (double)(decided + undecided) * 100.0);

    free_graph(g);
    free_graph(g1);
    free_graph(g2);
    for (int i = 0; i < N; i++)
        free_graph(cc[i]);
    free(cc);

    return 0;
}