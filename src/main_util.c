#include "graph.h"
#include "ocm.h"
#include "heuristics.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    graph gz = remove_degree_zero(g);

    int N;
    graph *cc = split_graph(gz, &N);
    for (int i = 0; i < N; i++)
    {
        if (cc[i].B < 2)
            continue;

        ocm p = init_ocm_problem(cc[i]);
        ocm_reduction_trivial(&p);
        ocm_reduction_twins(cc[i], &p);
        ocm_reduction_2_1(cc[i], &p);
        ocm_reduction_independent(&p);

        int *s = malloc(sizeof(int) * p.N);
        int ub = simulated_annealing(cc[i], s);
        free(s);

        ocm_reduction_k_full(&p, ub);

        printf("%d %d (%d %d) %d %d %d %d\n", p.N, p.crossings + p.lb, p.crossings, p.lb, ub, p.equal, p.undecided, count_relevant_vertices(p));

        free_ocm_problem(p);
    }

    free_graph(g);
    free_graph(gz);
    return 0;
}