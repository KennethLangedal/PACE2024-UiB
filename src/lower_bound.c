#include "lower_bound.h"

#include <stdlib.h>

int lower_bound_greedy(comp c)
{
    int back_edges = 0, min_in_cycle = 0;
    for (int i = 0; i < c.n; i++)
    {
        int u = c.S[i];
        for (int j = i + 1; j < c.n; j++)
        {
            int v = c.S[j];
            if (c.W[v][u] > 0)
            {
                back_edges++;
                for (int k = i + 1; k < j; k++)
                {
                    int w = c.S[k];
                    if (c.W[v][u] <= c.W[u][w] && c.W[v][u] <= c.W[w][v])
                    {
                        min_in_cycle++;
                        break;
                    }
                }
            }
        }
    }
    printf("%d back edges (%d optimal edge in some cycle)\n", back_edges, min_in_cycle);

    return 0;
}