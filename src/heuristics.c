#include "heuristics.h"
#include "ocm.h"

#include <stdlib.h>
#include <math.h>

int simulated_annealing_cm(int **cm, int **tc, int N)
{
    int *s = malloc(sizeof(int) * N);
    for (int i = 0; i < N; i++)
        s[i] = i;

    if (N < 2)
        return 0;

    int64_t cost = 0;
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            cost += cm[s[i]][s[j]];

    double t = 0.9;
    int it = 200;

    while (t > 0.001)
    {
        for (int k = 0; k < it; k++)
        {
            int i = rand() % (N - 1);
            int64_t current_cost = cm[s[i]][s[i + 1]];
            int64_t alt_cost = cm[s[i + 1]][s[i]];
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

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            tc[i][j] = 0;

    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            tc[s[i]][s[j]] = 1;

    free(s);

    return cost;
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
            cost += number_of_crossings(g, s[i], s[j]);

    double t = 0.9;
    int it = 200;

    while (t > 0.001)
    {
        for (int k = 0; k < it; k++)
        {
            int i = rand() % (g.B - 1);
            int64_t current_cost = number_of_crossings(g, s[i], s[i + 1]);
            int64_t alt_cost = number_of_crossings(g, s[i + 1], s[i]);
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