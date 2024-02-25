#include "heuristics.h"
#include "ocm.h"

#include <stdlib.h>
#include <math.h>

void ocm_simulated_annealing(ocm *p)
{
    int *s = malloc(sizeof(int) * p->N);
    for (int i = 0; i < p->N; i++)
        s[i] = i;

    if (p->N < 2)
        return;

    int64_t cost = 0;
    for (int i = 0; i < p->N; i++)
        for (int j = i + 1; j < p->N; j++)
            cost += p->cm[s[i]][s[j]];

    double t = 0.9;
    int it = 200;

    while (t > 0.001)
    {
        for (int k = 0; k < it; k++)
        {
            int i = rand() % (p->N - 1);
            int64_t current_cost = p->cm[s[i]][s[i + 1]];
            int64_t alt_cost = p->cm[s[i + 1]][s[i]];
            double diff = current_cost - alt_cost;
            if (diff >= 0 || exp(diff / t) > drand48())
            {
                cost -= current_cost;
                cost += alt_cost;
                int t = s[i];
                s[i] = s[i + 1];
                s[i + 1] = t;
            }
        }
        t *= 0.9999;
    }

    for (int i = 0; i < p->N; i++)
        for (int j = 0; j < p->N; j++)
            p->tc[i][j] = 0;

    for (int i = 0; i < p->N; i++)
        for (int j = i + 1; j < p->N; j++)
            p->tc[s[i]][s[j]] = 1;

    free(s);

    p->equal = 0;
    p->undecided = 0;
    p->crossings = cost;
    p->lb = 0;
}

int simulated_annealing(graph g, int *s)
{
    for (int i = g.A; i < g.N; i++)
        s[i - g.A] = i;

    if (g.B < 2)
        return 0;

    int64_t cost = 0;
    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            cost += count_crossings_pair(g, s[i], s[j]);

    double t = 0.9;
    int it = 400;

    while (t > 0.001)
    {
        for (int k = 0; k < it; k++)
        {
            int i = rand() % (g.B - 1);
            int64_t current_cost = count_crossings_pair(g, s[i], s[i + 1]);
            int64_t alt_cost = count_crossings_pair(g, s[i + 1], s[i]);
            double diff = current_cost - alt_cost;
            if (diff >= 0 || exp(diff / t) > drand48())
            {
                cost -= current_cost;
                cost += alt_cost;
                int t = s[i];
                s[i] = s[i + 1];
                s[i + 1] = t;
            }
        }
        t *= 0.9999;
    }

    return cost;
}