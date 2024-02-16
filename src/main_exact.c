// #include "graph.h"
// #include "ocm.h"

// #include <stdlib.h>
// #include <stdio.h>

// int lb(int **cm, int **tc, int N)
// {
//     int res = 0;
//     for (int i = 0; i < N; i++)
//     {
//         for (int j = i + 1; j < N; j++)
//         {
//             if (cm[i][j] != cm[j][i] && tc[i][j] == tc[j][i])
//                 res += cm[i][j] < cm[j][i] ? cm[i][j] : cm[j][i];
//         }
//     }
//     return res;
// }

// int bc = 0;
// int lr[200];

// void bb(int **cm, int **tc, int N, int crossings, int **sol, int *ub, int d)
// {
//     if (crossings + lb(cm, tc, N) >= *ub)
//         return;

//     // find pair
//     int u = -1, v = -1;
//     for (int i = 0; i < N && u < 0; i++)
//         for (int j = i + 1; j < N && v < 0; j++)
//             if (tc[i][j] == 0 && tc[j][i] == 0 && cm[i][j] != cm[j][i])
//                 u = i, v = j;

//     if (u < 0 && v < 0)
//     {
//         if (crossings < *ub)
//         {
//             *ub = crossings;
//             for (int i = 0; i < N * N; i++)
//                 sol[0][i] = tc[0][i];
//             printf("%d\n", crossings);
//         }
//         return;
//     }

//     bc++;
//     lr[d] = 0;
//     if (!(bc % 100))
//     {
//         printf("\r");
//         for (int i = 0; i < 200; i++)
//             printf("%d", lr[i]);
//         fflush(stdout);
//     }

//     int **_tc = init_tc(N);

//     // u < v branch
//     int b1 = tc_add_edge(tc, _tc, N, u, v, cm) + crossings;
//     bb(cm, _tc, N, b1, sol, ub, d + 1);

//     lr[d] = 1;
//     // v < u branch
//     int b2 = tc_add_edge(tc, _tc, N, v, u, cm) + crossings;
//     bb(cm, _tc, N, b2, sol, ub, d + 1);

//     free_tc(_tc);
// }

// void greedy(int **cm, int **tc, int N, int *C)
// {
//     for (int i = 0; i < N; i++)
//     {
//         for (int j = i + 1; j < N; j++)
//         {

//             if (tc[i][j] == 0 && tc[j][i] == 0 && cm[i][j] != cm[j][i])
//             {
//                 if (cm[i][j] < cm[j][i])
//                     *C += tc_add_edge(tc, tc, N, i, j, cm);
//                 else
//                     *C += tc_add_edge(tc, tc, N, j, i, cm);
//             }
//         }
//     }
// }

#include "bnb.h"
#include "graph.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    int *s = solve_bnb(g);
    f = fopen(argv[2], "w");
    for (int i = 0; i < g.B; i++)
        fprintf(f, "%d\n", s[i] + 1);
    fclose(f);

    free_graph(g);
    free(s);

    return 0;
}