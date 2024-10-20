#include "ocm.h"
#include "dfas.h"
#include "tiny_solver.h"
#include "heuristics.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// #define ARG_IO

volatile sig_atomic_t tle = 0;

void term(int signum)
{
    tle = 1;
}

int main(int argc, char **argv)
{
    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = term;
    sigaction(SIGTERM, &action, NULL);

#ifdef ARG_IO
    FILE *f = fopen(argv[1], "r");
    ocm p = ocm_parse(f);
    fclose(f);
#else
    ocm p = ocm_parse(stdin);
#endif

    dfas g = dfas_construct(p);
    int valid = g.n > 0;
    for (int i = 0; i < g.n; i++)
        if (g.C[i].n < 0)
            valid = 0;

    int *R;

    if (!valid)
    {
        // fprintf(stderr, "Failed\n");
        R = ocm_average_placement(p);
        int cost = 0;
        ocm_greedy_improvement(p, R, &cost, &tle);
    }
    else
    {
        int solved = 1, cost = 0;
        int *_S = malloc(sizeof(int) * p.n1);
        int **S = malloc(sizeof(int *) * (g.n + 1));
        int *B = malloc(sizeof(int) * g.n);

        int m = 0;
        int *P = malloc(sizeof(int) * g.n);

        S[0] = _S;
        for (int i = 0; i < g.n; i++)
        {
            comp c = g.C[i];
            S[i + 1] = S[i] + c.n;
            if (c.n > 1 && c.n <= 20)
                tiny_solver_solve(c);
            else if (c.n > 20)
            {
                P[m++] = i;
                B[i] = *c.c;
                for (int j = 0; j < c.n; j++)
                    S[i][j] = c.S[j];
            }
            cost += *c.c;
        }
        // fprintf(stderr, "%d %d\n", cost + g.offset, m);

        int t = 0;
        while (!tle)
        {
            for (int _i = 0; _i < m && !tle; _i++)
            {
                int i = P[_i];
                comp c = g.C[i];

                heuristic_randomize_solution(c, (rand() % 3) + 1);
                heuristics_greedy_improvement(c, &tle);
                tiny_solver_sliding_solve(c, 8);

                int old_c = *c.c + 1;
                while (old_c > *c.c && !tle && *c.c < B[i] + 20)
                {
                    old_c = *c.c;
                    heuristics_greedy_cut(c, &tle);
                }

                if (*c.c < B[i])
                {
                    for (int j = 0; j < c.n; j++)
                        S[i][j] = c.S[j];

                    cost -= B[i];
                    B[i] = *c.c;
                    cost += B[i];
                }
                else if (*c.c > B[i])
                {
                    *c.c = B[i];
                    for (int j = 0; j < c.n; j++)
                        c.S[j] = S[i][j];
                }
            }
            // fprintf(stderr, "%d %d\n", cost + g.offset, t++);
        }
        R = dfas_get_solution(p, g);
        free(_S);
        free(S);
        free(B);
        free(P);
    }

#ifdef ARG_IO
    f = fopen(argv[2], "w");
    for (int i = 0; i < p.n1; i++)
        fprintf(f, "%d\n", p.n0 + R[i] + 1);
    fclose(f);
#else
    for (int i = 0; i < p.n1; i++)
        printf("%d\n", p.n0 + R[i] + 1);
#endif

    ocm_free(p);
    dfas_free(g);
    free(R);
    return 0;
}