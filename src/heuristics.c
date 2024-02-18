#include "heuristics.h"
#include "ocm.h"

#include <stdlib.h>
#include <math.h>

void move_left(int **cm, int *cost, int *s, int i)
{
    *cost -= cm[s[i - 1]][s[i]];
    *cost += cm[s[i]][s[i - 1]];
    int t = s[i];
    s[i] = s[i - 1];
    s[i - 1] = t;
}

void move_right(int **cm, int *cost, int *s, int i)
{
    *cost -= cm[s[i]][s[i + 1]];
    *cost += cm[s[i + 1]][s[i]];
    int t = s[i];
    s[i] = s[i + 1];
    s[i + 1] = t;
}

int greedy_placement(int N, int **cm)
{
    int *s = malloc(sizeof(int) * N);

    for (int i = 0; i < N; i++)
        s[i] = i;

    int cost = 0;
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++)
            cost += cm[i][j];

    double t = 0.9;

    while (t > 0.001)
    {
        int i = rand() % (N - 1);
        double diff = cm[s[i]][s[i + 1]] - cm[s[i + 1]][s[i]];
        if (diff >= 0 || exp(diff / t) > drand48())
        {
            move_right(cm, &cost, s, i);
        }
        t *= 0.99999;
    }

    free(s);
    return cost;
}

int simulated_annealing(graph g, int *s)
{
    for (int i = g.A; i < g.N; i++)
        s[i - g.A] = i;

    int cost = 0;
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
            int current_cost = number_of_crossings(g, s[i], s[i + 1]);
            int alt_cost = number_of_crossings(g, s[i + 1], s[i]);
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
        t *= 0.99999;
    }

    return cost;
}