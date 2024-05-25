#include "ocm.h"
#include "dfas.h"
#include "heuristics.h"
#include "tiny_solver.h"
#include "lower_bound.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    ocm p = ocm_parse(f);
    fclose(f);

    dfas g = dfas_construct(p);
    if (g.n < 0)
    {
        fprintf(stderr, "%s, failed\n", argv[1]);
        f = fopen(argv[2], "w");
        for (int i = 0; i < p.n1; i++)
            fprintf(f, "%d\n", p.n0 + i + 1);
        fclose(f);
    }

    int max_c = 0;
    for (int i = 0; i < g.n; i++)
        if (g.C[i].n > max_c)
            max_c = g.C[i].n;

    int solved = 1, cost = 0;
    for (int i = 0; i < g.n; i++)
    {
        comp c = g.C[i];
        if (c.n > 1 && c.n <= 20)
            tiny_solver_solve(c);
        else if (c.n > 20)
        {
            solved = 0;

            int *best = malloc(sizeof(int) * c.n);
            for (int j = 0; j < c.n; j++)
                best[j] = c.S[j];
            int best_cost = *c.c;

            for (int t = 0; t < 50; t++)
            {
                heuristic_randomize_solution(c, (rand() % 4) + 1);
                heuristics_greedy_improvement(c);
                tiny_solver_sliding_solve(c, 8);

                int old_c = *c.c + 1;
                while (old_c > *c.c)
                {
                    old_c = *c.c;
                    heuristics_greedy_cut(c);
                }

                if (*c.c < best_cost)
                {
                    for (int j = 0; j < c.n; j++)
                        best[j] = c.S[j];
                    best_cost = *c.c;
                    printf("%d %d\n", t, *c.c);
                }
                else if (*c.c > best_cost)
                {
                    *c.c = best_cost;
                    for (int j = 0; j < c.n; j++)
                        c.S[j] = best[j];
                }
            }
            printf("%d\n", lower_bound_greedy(c));
        }

        cost += *c.c;
    }

    // fprintf(stderr, "%d\n", g.offset);
    int *S = dfas_get_solution(p, g);
    f = fopen(argv[2], "w");
    for (int i = 0; i < p.n1; i++)
        fprintf(f, "%d\n", p.n0 + S[i] + 1);
    fclose(f);
    free(S);

    fprintf(stderr, "%s, opt=%d, num_c=%d, max_c=%d, solution=%d\n", argv[1], solved, g.n, max_c, g.offset + cost);

    ocm_free(p);
    dfas_free(g);
    return 0;
}