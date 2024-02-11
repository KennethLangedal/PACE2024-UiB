#include "ocm.h"

#include <stdlib.h>

int **ocm_compute_cost_matrix(int N, const int *V, const int *E, int N0, int N1)
{
    int *data = malloc(sizeof(int) * N1 * N1);
    int **cm = malloc(sizeof(int *) * N1);

    for (int i = 0; i < N1; i++)
        cm[i] = data + i * N1;

    for (int u = 0; u < N1; u++)
    {
        for (int v = 0; v < N1; v++)
        {
            cm[u][v] = 0;
            if (u == v)
                continue;

            int i = V[N0 + u];
            int j = V[N0 + v];

            while (i < V[N0 + u + 1] && j < V[N0 + v + 1])
            {
                if (E[j] < E[i])
                {
                    cm[u][v] += V[N0 + u + 1] - i;
                    j++;
                }
                else
                    i++;
            }
        }
    }

    return cm;
}

void ocm_free_cost_matrix(int **cm)
{
    free(cm[0]);
    free(cm);
}