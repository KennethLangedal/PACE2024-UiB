#include "ocm.h"
#include "dfas.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    ocm p = ocm_parse(stdin);

    dfas g = dfas_construct(p, 2000000);
    if (g.n < 0)
    {
        fprintf(stderr, "Aborted\n");
    }
    else
    {
        int *S = malloc(sizeof(int) * g.V[g.n]);
        for (int i = 0; i < g.V[g.n]; i++)
            S[i] = 0;

        scc_split cc = dfas_scc_split(g);

        int solved = 1, max = 0;
        for (int i = 0; i < cc.n; i++)
        {
            if (cc.C[i].n > 2)
                solved = 0;
            if (cc.C[i].n > cc.C[max].n)
                max = i;
        }

        if (solved)
        {
            fprintf(stderr, "Graph solver by scc %d %d\n", g.n, g.V[g.n]);
            int *res = dfas_lift_solution(p, g, S);
            for (int i = 0; i < g.n; i++)
                printf("%d\n", p.n0 + res[i] + 1);
            free(res);
        }
        else
        {
            fprintf(stderr, "Max component %d %d\n", cc.C[max].n, cc.C[max].V[cc.C[max].n]);
        }

        free(S);
        dfas_free_scc_split(cc);
    }

    ocm_free(p);
    dfas_free(g);
    return 0;
}