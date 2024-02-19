#include "graph.h"
#include "ocm.h"
#include "bnb.h"
#include "heuristics.h"

#include <stdlib.h>
#include <stdio.h>

static inline int natural_order(int **cm, int N, int u, int v)
{
    if (cm[u][v] == 0 || cm[v][u] == 0)
        return 0;
    return cm[u][v] < cm[v][u];
}

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    printf("%s %d ", argv[1], g.B);
    graph gz = remove_degree_zero(g);
    printf("%d ", gz.B);
    graph gt = remove_twins(gz);
    printf("%d ", gt.B);

    int N = 0;
    graph *cc = split_graph(gt, &N);

    int max = 0;
    for (int i = 0; i < N; i++)
        if (cc[i].B > cc[max].B)
            max = i;

    int **cm = init_cost_matrix(cc[max]);
    int **tc = init_tc(cc[max].B);
    int lb = lower_bound(cm, tc, cc[max].B);
    int lb_imp = lower_bound_improved(cm, tc, cc[max].B);
    int ub = simulated_annealing_cm(cm, tc, cc[max].B);

    printf("%d %d %d %d\n", cc[max].B, lb, lb_imp, ub);

    return 0;
}