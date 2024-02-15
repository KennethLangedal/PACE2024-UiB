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

int main(int argc, char **argv)
{
}

// int main(int argc, char **argv)
// {
//     int N, N0, N1;
//     int *V = NULL, *E = NULL;

//     graph_parse(stdin, &N, &N0, &N1, &V, &E);

//     // if (graph_validate(N, N0, N1, V, E))
//     //     printf("|V|=%d, |E|=%d, |A|=%d, |B|=%d\n", N, V[N], N0, N1);
//     // else
//     //     printf("Error in graph\n");

//     int _N = 0;
//     int **cm = compute_cost_matrix_relevant(N, V, E, N0, N1, &_N);
//     int **tc = init_tc(_N);

//     // printf("|N| = %d\n", _N);

//     int *new_label = malloc(sizeof(int) * N1);
//     int *new_label_fixed = malloc(sizeof(int) * N0);
//     for (int i = 0; i < N0; i++)
//         new_label_fixed[i] = -1;

//     int t = 0, t2 = 0;
//     for (int i = 0; i < N1; i++)
//     {
//         if (V[N0 + i + 1] - V[N0 + i] > 1)
//             new_label[i] = t++;
//         else
//         {
//             new_label[i] = -1;
//             continue;
//         }

//         for (int j = V[N0 + i]; j < V[N0 + i + 1]; j++)
//             if (new_label_fixed[E[j]] < 0)
//                 new_label_fixed[E[j]] = t2++;
//     }

//     int ec = 0;
//     for (int i = 0; i < N1; i++)
//     {
//         if (new_label[i] < 0)
//             continue;
//         ec += V[N0 + i + 1] - V[N0 + i];
//     }
//     printf("p ocr %d %d %d\n", t2, t, ec);
//     for (int i = 0; i < N1; i++)
//     {
//         if (new_label[i] < 0)
//             continue;
//         for (int j = V[N0 + i]; j < V[N0 + i + 1]; j++)
//         {
//             printf("%d %d\n", new_label_fixed[E[j]] + 1, new_label[i] + 1 + t2);
//         }
//     }

//     // for (int i = 0; i < _N; i++)
//     // {
//     //     for (int j = i + 1; j < _N; j++)
//     //     {
//     //         if (cm[i][j] == 0)
//     //             tc[i][j] = 1;
//     //         else if (cm[j][i] == 0)
//     //             tc[j][i] = 1;
//     //     }
//     // }

//     // int problem_pairs = 0;
//     // int known_crossings = 0;
//     // int lb = 0;
//     // int ub = 0;

//     // for (int i = 0; i < _N; i++)
//     // {
//     //     for (int j = i + 1; j < _N; j++)
//     //     {
//     //         if (cm[i][j] == 0 || cm[j][i] == 0)
//     //             continue;

//     //         if (cm[i][j] != cm[j][i])
//     //         {
//     //             lb += cm[i][j] < cm[j][i] ? cm[i][j] : cm[j][i];
//     //             ub += cm[i][j] < cm[j][i] ? cm[j][i] : cm[i][j];
//     //             problem_pairs++;
//     //         }
//     //         else
//     //         {
//     //             known_crossings += cm[i][j];
//     //         }
//     //     }
//     // }

//     // printf("problem: %d, lb: %d, ub: %d, known: %d\n",
//     //        problem_pairs, lb, ub, known_crossings);

//     // int C = 0;
//     // int **sol = init_tc(_N);
//     // greedy(cm, sol, _N, &C);

//     // printf("UB: %d\n", C);

//     // bb(cm, tc, _N, 0, sol, &C, 0);

//     // printf("%d\n", bc);

//     // int easy_pairs = 0;

//     // for (int i = 0; i < N1; i++)
//     // {
//     //     for (int j = i + 1; j < N1; j++)
//     //     {
//     //         if (cm[i][j] == 0 || cm[j][i] == 0)
//     //             easy_pairs++;
//     //     }
//     // }

//     // printf("%d/%d (%.2lf%%) easy\n", easy_pairs, (N1 * (N1 - 1)) / 2,
//     //        (double)easy_pairs / (double)((N1 * (N1 - 1)) / 2) * 100.0);

//     // free_cost_matrix(cm);
//     // free_tc(tc);

//     free(V);
//     free(E);

//     return 0;
// }