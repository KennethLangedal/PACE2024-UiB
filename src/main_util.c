#include "gr.h"
#include "ocm.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int N, N0, N1;
    int *V = NULL, *E = NULL;

    gr_parse(stdin, &N, &N0, &N1, &V, &E);

    if (gr_validate(N, N0, N1, V, E))
        printf("Valid graph. |V|=%d, |E|=%d, |A|=%d, |B|=%d\n", N, V[N], N0, N1);
    else
        printf("Error in graph\n");

    // int **cm = ocm_compute_cost_matrix(N, V, E, N0, N1);

    // int easy_pairs = 0;

    // for (int i = 0; i < N1; i++)
    // {
    //     for (int j = i + 1; j < N1; j++)
    //     {
    //         if (cm[i][j] == 0 || cm[j][i] == 0)
    //             easy_pairs++;
    //     }
    // }

    // printf("%d/%d (%.2lf%%) easy\n", easy_pairs, (N1 * (N1 - 1)) / 2,
    //        (double)easy_pairs / (double)((N1 * (N1 - 1)) / 2) * 100.0);

    // ocm_free_cost_matrix(cm);

    free(V);
    free(E);

    return 0;
}