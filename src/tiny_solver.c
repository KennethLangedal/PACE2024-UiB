#include "tiny_solver.h"

#include <stdlib.h>
#include <immintrin.h>

#define DIM 24

void fill_dp_avx(int *DP, int **W, int n)
{
    DP[0] = 0;
    int *i_vec = aligned_alloc(32, sizeof(int) * DIM);
    for (int i = 1; i < (1 << n); i++)
    {
        for (int j = 0; j < DIM; j++)
            i_vec[j] = (i & (1 << j)) ? 0xFFFFFFFF : 0;

        int best = 9999999;
        for (int j = 0; j < DIM; j++)
        {
            if (i_vec[j] == 0)
                continue;

            i_vec[j] = 0;
            __m256i m0 = _mm256_load_si256((__m256i *)(i_vec + 0));
            __m256i m1 = _mm256_load_si256((__m256i *)(i_vec + 8));
            __m256i m2 = _mm256_load_si256((__m256i *)(i_vec + 16));

            __m256i w0 = _mm256_load_si256((__m256i *)(W[j] + 0));
            __m256i w1 = _mm256_load_si256((__m256i *)(W[j] + 8));
            __m256i w2 = _mm256_load_si256((__m256i *)(W[j] + 16));

            w0 = _mm256_and_si256(m0, w0);
            w1 = _mm256_and_si256(m1, w1);
            w2 = _mm256_and_si256(m2, w2);

            w0 = _mm256_hadd_epi32(w0, w1);
            w0 = _mm256_hadd_epi32(w0, w2);

            w0 = _mm256_hadd_epi32(w0, w0);
            w0 = _mm256_hadd_epi32(w0, w0);

            int c = _mm256_extract_epi32(w0, 0) + _mm256_extract_epi32(w0, 4);

            i_vec[j] = 0xFFFFFFFF;

            int _i = i ^ (1 << j);
            if (DP[_i] + c < best)
                best = DP[_i] + c;
        }
        DP[i] = best;
    }

    free(i_vec);
}

void tiny_solver_solve(comp c)
{
    int *DP = malloc(sizeof(int) * (1 << c.n));
    int *aligned_data = aligned_alloc(32, sizeof(int) * DIM * DIM);
    int **W = malloc(sizeof(int *) * DIM);

    for (int i = 0; i < DIM; i++)
        W[i] = aligned_data + i * DIM;

    for (int i = 0; i < DIM * DIM; i++)
        aligned_data[i] = 0;

    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            W[i][j] = c.W[i][j];

    fill_dp_avx(DP, W, c.n);

    int i = (1 << c.n) - 1;
    *c.c = DP[i];
    int p = c.n - 1;
    while (i > 0)
    {
        for (int j = 0; j < DIM; j++)
        {
            if (!(i & (1 << j)))
                continue;

            int _i = i ^ (1 << j);
            int cr = 0;
            for (int k = 0; k < DIM; k++)
                if ((1 << k) & _i)
                    cr += W[j][k];

            if (DP[_i] + cr == DP[i])
            {
                c.S[p--] = j;
                i ^= 1 << j;
                break;
            }
        }
    }
    free(DP);
    free(aligned_data);
    free(W);
}

void tiny_solver_sliding_solve(comp c, int size)
{
    int buffer[DIM];
    for (int i = 0; i < size; i++)
        buffer[i] = c.S[c.n - size + i];

    int *DP = malloc(sizeof(int) * (1 << size));
    int *aligned_data = aligned_alloc(32, sizeof(int) * DIM * DIM);
    int **W = malloc(sizeof(int *) * DIM);

    for (int i = 0; i < DIM; i++)
        W[i] = aligned_data + i * DIM;

    for (int i = 0; i < DIM * DIM; i++)
        aligned_data[i] = 0;

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            W[i][j] = c.W[buffer[i]][buffer[j]];

    fill_dp_avx(DP, W, size);

    for (int t = c.n - 1; t >= size; t--)
    {
        int i = (1 << size) - 1, _j = 0;
        for (int j = 0; j < size; j++)
        {
            int _i = i ^ (1 << j);
            int cr = 0;
            for (int k = 0; k < size; k++)
                if ((1 << k) & _i)
                    cr += W[j][k];

            if (DP[_i] + cr == DP[i])
            {
                _j = j;
                // c.S[t] = buffer[j];
                // break;
            }
        }
        c.S[t] = buffer[_j];
        // Repopulate with next element replacing j
        buffer[_j] = c.S[t - size];
        for (int i = 0; i < size; i++)
        {
            W[i][_j] = c.W[buffer[i]][buffer[_j]];
            W[_j][i] = c.W[buffer[_j]][buffer[i]];
        }

        fill_dp_avx(DP, W, size);
    }

    int i = (1 << size) - 1;
    int t = size - 1;
    while (i > 0)
    {
        for (int j = 0; j < size; j++)
        {
            if (!(i & (1ll << j)))
                continue;

            int _i = i ^ (1ll << j);
            int cr = 0;
            for (int k = 0; k < DIM; k++)
                if ((1 << k) & _i)
                    cr += W[j][k];

            if (DP[_i] + cr == DP[i])
            {
                c.S[t--] = buffer[j];
                i ^= 1 << j;
                break;
            }
        }
    }

    *c.c = 0;
    for (int i = 0; i < c.n; i++)
        for (int j = i + 1; j < c.n; j++)
            *c.c += c.W[c.S[j]][c.S[i]];

    free(DP);
    free(aligned_data);
    free(W);
}

// tiny_solver tiny_solver_init()
// {
//     tiny_solver s;
//     s.Data = aligned_alloc(32, sizeof(int) * DIM * DIM);
//     s.W = malloc(sizeof(int *) * DIM);
//     for (int i = 0; i < DIM; i++)
//         s.W[i] = s.Data + i * DIM;

//     s.DP = malloc(sizeof(int) * (1ll << MAX_SIZE));
//     return s;
// }

// void tiny_solver_free(tiny_solver s)
// {
//     free(s.Data);
//     free(s.W);
//     free(s.DP);
// }

// void fill_dp(tiny_solver s, int n)
// {
//     s.DP[0] = 0;
//     for (int64_t i = 1; i < (1ll << n); i++)
//     {
//         int best = 9999999;
//         for (int64_t j = 0; j < DIM; j++)
//         {
//             if (!(i & (1ll << j)))
//                 continue;

//             int64_t _i = i ^ (1ll << j);
//             int c = 0;
//             for (int k = 0; k < DIM; k++)
//                 if ((1ll << k) & _i)
//                     c += s.W[j][k];

//             if (s.DP[_i] + c < best)
//                 best = s.DP[_i] + c;
//         }
//         s.DP[i] = best;
//     }
// }

// int *tiny_solver_solve(tiny_solver s, dfas g)
// {
//     for (int i = 0; i < DIM * DIM; i++)
//         s.Data[i] = 0;

//     for (int u = 0; u < g.n; u++)
//         for (int i = g.V[u]; i < g.V[u + 1]; i++)
//             s.W[u][g.E[i]] = g.W[i];

//     fill_dp_avx(s, g.n);

//     int *S = malloc(sizeof(int) * g.V[g.n]);
//     for (int i = 0; i < g.V[g.n]; i++)
//         S[i] = 0;

//     int64_t i = (1ll << g.n) - 1ll;
//     while (i > 0)
//     {
//         for (int64_t j = 0; j < DIM; j++)
//         {
//             if (!(i & (1ll << j)))
//                 continue;

//             int64_t _i = i ^ (1ll << j);
//             int c = 0;
//             for (int64_t k = 0; k < DIM; k++)
//                 if ((1ll << k) & _i)
//                     c += s.W[j][k];

//             if (s.DP[_i] + c == s.DP[i])
//             {
//                 for (int64_t k = g.V[j]; k < g.V[j + 1]; k++)
//                     if (_i & (1ll << g.E[k]))
//                         S[k] = 1;
//                 i ^= 1ll << j;
//                 break;
//             }
//         }
//     }

//     return S;
// }

// void tiny_solver_sliding_solve(tiny_solver s, int **W, int *O, int n, int size)
// {
//     int buffer[DIM];
//     for (int i = 0; i < size; i++)
//         buffer[i] = O[n - size + i];

//     for (int i = 0; i < DIM * DIM; i++)
//         s.Data[i] = 0;

//     for (int i = 0; i < size; i++)
//         for (int j = 0; j < size; j++)
//             s.W[i][j] = W[buffer[i]][buffer[j]];

//     fill_dp_avx(s, size);

//     for (int t = n - 1; t >= size; t--)
//     {
//         int64_t i = (1ll << size) - 1ll, j = 0;
//         for (j = 0; j < size; j++)
//         {
//             int64_t _i = i ^ (1ll << j);
//             int c = 0;
//             for (int64_t k = 0; k < size; k++)
//                 if ((1ll << k) & _i)
//                     c += s.W[j][k];

//             if (s.DP[_i] + c == s.DP[i])
//             {
//                 O[t] = buffer[j];
//                 break;
//             }
//         }

//         // Repopulate with next element replacing j
//         buffer[j] = O[t - size];
//         for (int i = 0; i < size; i++)
//         {
//             s.W[i][j] = W[buffer[i]][buffer[j]];
//             s.W[j][i] = W[buffer[j]][buffer[i]];
//         }

//         fill_dp_avx(s, size);
//     }

//     int64_t i = (1ll << size) - 1ll;
//     int t = size - 1;
//     while (i > 0)
//     {
//         for (int64_t j = 0; j < size; j++)
//         {
//             if (!(i & (1ll << j)))
//                 continue;

//             int64_t _i = i ^ (1ll << j);
//             int c = 0;
//             for (int64_t k = 0; k < DIM; k++)
//                 if ((1ll << k) & _i)
//                     c += s.W[j][k];

//             if (s.DP[_i] + c == s.DP[i])
//             {
//                 O[t--] = buffer[j];
//                 i ^= 1ll << j;
//                 break;
//             }
//         }
//     }
// }

// int tiny_solver_mask_solve(tiny_solver s, dfas g, int *new_label, int *mask)
// {
//     for (int i = 0; i < DIM * DIM; i++)
//         s.Data[i] = 0;

//     int id = 0;
//     for (int u = 0; u < g.n; u++)
//         if (mask[u])
//             new_label[u] = id++;

//     for (int u = 0; u < g.n; u++)
//         if (mask[u])
//             for (int i = g.V[u]; i < g.V[u + 1]; i++)
//                 if (mask[g.E[i]])
//                     s.W[new_label[u]][new_label[g.E[i]]] = g.W[i];

//     fill_dp_avx(s, id);

//     return s.DP[(1ll << id) - 1ll];
// }

// int tiny_solver_mask_solve_alt(tiny_solver s, dfas g, dfas gt, int *new_label, int *mask)
// {
//     for (int i = 0; i < DIM * DIM; i++)
//         s.Data[i] = 0;

//     int bo[DIM], bi[DIM];
//     for (int i = 0; i < DIM; i++)
//     {
//         bo[i] = 0;
//         bi[i] = 0;
//     }

//     int id = 0;
//     for (int u = 0; u < g.n; u++)
//     {
//         if (!mask[u])
//             continue;

//         new_label[u] = id++;
//         for (int i = g.V[u]; i < g.V[u + 1]; i++)
//         {
//             if (!mask[g.E[i]])
//             {
//                 bo[new_label[u]] = 1;
//                 break;
//             }
//         }

//         for (int i = gt.V[u]; i < gt.V[u + 1]; i++)
//         {
//             if (!mask[gt.E[i]])
//             {
//                 bi[new_label[u]] = 1;
//                 break;
//             }
//         }

//         if (bo[new_label[u]] && bi[new_label[u]])
//             id++;

//         if (id > MAX_SIZE)
//             return -1;
//     }

//     for (int u = 0; u < g.n; u++)
//     {
//         if (!mask[u])
//             continue;

//         int _u = new_label[u];
//         if (bo[_u] && bi[_u])
//             _u++;

//         for (int i = g.V[u]; i < g.V[u + 1]; i++)
//             if (mask[g.E[i]])
//                 s.W[_u][new_label[g.E[i]]] = g.W[i];

//         _u = new_label[u];
//         if (bo[_u])
//         {
//             for (int i = 0; i < DIM; i++)
//                 if (i != _u && bi[i])
//                     s.W[_u][bo[i] ? i + 1 : i] = 999999;
//         }
//     }

//     fill_dp_avx(s, id);

//     return s.DP[(1ll << id) - 1ll];
// }

// int *tiny_solver_solve_order(tiny_solver s, dfas g)
// {
//     for (int i = 0; i < DIM * DIM; i++)
//         s.Data[i] = 0;

//     for (int u = 0; u < g.n; u++)
//         for (int i = g.V[u]; i < g.V[u + 1]; i++)
//             s.W[u][g.E[i]] = g.W[i];

//     fill_dp_avx(s, g.n);

//     int *S = malloc(sizeof(int) * g.n);

//     int64_t i = (1ll << g.n) - 1ll;
//     int p = g.n;
//     while (i > 0)
//     {
//         for (int64_t j = 0; j < DIM; j++)
//         {
//             if (!(i & (1ll << j)))
//                 continue;

//             int64_t _i = i ^ (1ll << j);
//             int c = 0;
//             for (int64_t k = 0; k < DIM; k++)
//                 if ((1ll << k) & _i)
//                     c += s.W[j][k];

//             if (s.DP[_i] + c == s.DP[i])
//             {
//                 S[--p] = j;
//                 i ^= 1ll << j;
//                 break;
//             }
//         }
//     }

//     return S;
// }