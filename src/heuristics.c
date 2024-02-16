#include "heuristics.h"

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