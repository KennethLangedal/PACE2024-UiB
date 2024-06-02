#include "ocm.h"
#include "dfas.h"
#include "tiny_solver.h"
#include "lower_bound.h"
#include "heuristics.h"
#include "glpk.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    // glp_prob *lp;
    // int ia[1 + 1000], ja[1 + 1000];
    // double ar[1 + 1000];

    // lp = glp_create_prob();
    // glp_set_obj_dir(lp, GLP_MAX);

    // glp_add_rows(lp, 5);
    // glp_set_row_bnds(lp, 1, GLP_UP, 0.0, 1.0);
    // glp_set_row_bnds(lp, 2, GLP_UP, 0.0, 1.0);
    // glp_set_row_bnds(lp, 3, GLP_UP, 0.0, 1.0);
    // glp_set_row_bnds(lp, 4, GLP_UP, 0.0, 1.0);
    // glp_set_row_bnds(lp, 5, GLP_UP, 0.0, 2.0);

    // glp_add_cols(lp, 2);
    // glp_set_col_bnds(lp, 1, GLP_LO, 0.0, 0.0);
    // glp_set_obj_coef(lp, 1, 1.0);
    // glp_set_col_bnds(lp, 2, GLP_LO, 0.0, 0.0);
    // glp_set_obj_coef(lp, 2, 1.0);

    // ia[1] = 1, ja[1] = 1, ar[1] = 1.0;
    // ia[2] = 2, ja[2] = 1, ar[2] = 1.0;
    // ia[3] = 3, ja[3] = 2, ar[3] = 1.0;
    // ia[4] = 4, ja[4] = 2, ar[4] = 1.0;
    // ia[5] = 5, ja[5] = 1, ar[5] = 1.0;
    // ia[6] = 5, ja[6] = 2, ar[6] = 1.0;

    // glp_load_matrix(lp, 6, ia, ja, ar);

    // glp_simplex(lp, NULL);
    // double z = glp_get_obj_val(lp);

    // printf("%lf\n", z);

    // return 0;

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
                fprintf(stderr, "%d\n", c.n);
                int *best = malloc(sizeof(int) * c.n);
                for (int j = 0; j < c.n; j++)
                    best[j] = c.S[j];
                int best_cost = *c.c;

                for (int t = 0; t < 100; t++)
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
                        fprintf(stderr, "%d %d\n", t, *c.c);
                    }
                    else if (*c.c > best_cost)
                    {
                        *c.c = best_cost;
                        for (int j = 0; j < c.n; j++)
                            c.S[j] = best[j];
                    }
                }
                // fprintf(stderr, "%d %d\n", *c.c, lower_bound_cycle_packing(c));
                // for (int j = 0; j < 50; j++)
                //     fprintf(stderr, "%d %d\n", *c.c, lower_bound_cycle_packing(c));

                fprintf(stderr, "%d\n", *c.c + g.offset);
                lower_bound_cycle_packing_lp(c);

                // for (int j = 0; j < c.n; j++)
                // {
                //     printf("[%d", c.W[c.S[j]][c.S[0]]);
                //     for (int k = 1; k < c.n; k++)
                //         printf(", %d", c.W[c.S[j]][c.S[k]]);
                //     printf("],\n");
                // }
                // return 0;

                // if (!solve_lazy(c))
                //     solved = 0;
            }
            cost += *c.c;
        }

        fprintf(stderr, "%s %d\n", argv[1], cost + g.offset);

        if (solved)
        {
            int *S = dfas_get_solution(p, g);
            f = fopen(argv[2], "w");
            for (int i = 0; i < p.n1; i++)
                fprintf(f, "%d\n", p.n0 + S[i] + 1);
            fclose(f);
            free(S);
        }
        else
        {
            fprintf(stderr, "Failed to solve\n");
        }
    }

    ocm_free(p);
    dfas_free(g);
    return 0;
}