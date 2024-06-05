#include "ocm.h"
#include "dfas.h"
#include "tiny_solver.h"
#include "heuristics.h"
#include "exact.h"
#include "cycle_packing.h"

#include <stdio.h>
#include <stdlib.h>

volatile sig_atomic_t tle = 0;

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    ocm p = ocm_parse(f);
    fclose(f);
    dfas g = dfas_construct(p);
    int valid = g.n > 0;
    for (int i = 0; i < g.n; i++)
        if (g.C[i].n < 0)
            valid = 0;

    if (!valid)
    {
        fprintf(stderr, "%s, failed\n", argv[1]);
    }
    else
    {
        int solved = 1, cost = 0;
        for (int i = 0; i < g.n; i++)
        {
            comp c = g.C[i];
            if (c.n > 1 && c.n <= 20)
                tiny_solver_solve(c);
            else if (c.n > 20)
            {
                // fprintf(stderr, "%d\n", c.n);
                int *best = malloc(sizeof(int) * c.n);
                for (int j = 0; j < c.n; j++)
                    best[j] = c.S[j];
                int best_cost = *c.c;

                for (int t = 0; t < 20; t++)
                {
                    heuristic_randomize_solution(c, (rand() % 4) + 1);
                    heuristics_greedy_improvement(c);
                    tiny_solver_sliding_solve(c, 8);

                    int old_c = *c.c + 1;
                    while (old_c > *c.c)
                    {
                        old_c = *c.c;
                        heuristics_greedy_cut(c, &tle);
                    }

                    if (*c.c < best_cost)
                    {
                        for (int j = 0; j < c.n; j++)
                            best[j] = c.S[j];
                        best_cost = *c.c;
                        // fprintf(stderr, "%d %d\n", t, *c.c);
                    }
                    else if (*c.c > best_cost)
                    {
                        *c.c = best_cost;
                        for (int j = 0; j < c.n; j++)
                            c.S[j] = best[j];
                    }
                }
                free(best);

                fprintf(stderr, "%d %d\n", c.n, *c.c);

                packing p = cycle_packing_init(c);
                cycle_packing_greedy(p);
                fprintf(stderr, "%d %d\n", *c.c, *p.c);
                cycle_packing_free(p);
                solved = 0;
                
                // if (!solve_lazy(c))
                // {
                //     solved = 0;
                // }
            }
            cost += *c.c;
        }

        // fprintf(stderr, "%s %d\n", argv[1], cost + g.offset);

        // if (solved)
        // {
        //     int *S = dfas_get_solution(p, g);
        //     f = fopen(argv[2], "w");
        //     for (int i = 0; i < p.n1; i++)
        //         fprintf(f, "%d\n", p.n0 + S[i] + 1);
        //     fclose(f);
        //     free(S);
        // }
        // else
        // {
        //     fprintf(stderr, "Failed to solve\n");
        // }
    }

    ocm_free(p);
    dfas_free(g);
    return 0;
}