#include "heuristics.h"
#include "tiny_solver.h"

#include <stdlib.h>

void shuffle(int *array, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

int try_right(int **W, int *O, int *c, int i, int n, int *lr, int *ll)
{
    int u = O[i];
    int change = 0;
    int last_pos = i;
    int max_gain = lr[u];
    for (int j = i + 1; j < n && change - max_gain <= 0; j++)
    {
        int v = O[j];
        change += W[u][v];
        change -= W[v][u];
        max_gain -= W[v][u];
        if (change < 0)
        {
            last_pos = j;
            break;
        }
        if (change == 0)
            last_pos = j;
    }
    if (last_pos != i)
    {
        if (change < 0)
            *c += change;
        for (int k = i; k < last_pos; k++)
        {
            O[k] = O[k + 1];
            lr[O[k]] += W[u][O[k]];
            ll[O[k]] -= W[O[k]][u];

            lr[u] -= W[O[k]][u];
            ll[u] += W[u][O[k]];
        }
        O[last_pos] = u;
    }
    return change < 0;
}

int try_left(int **W, int *O, int *c, int i, int n, int *lr, int *ll)
{
    int u = O[i];
    int change = 0;
    int last_pos = i;
    int max_gain = ll[u];
    for (int j = i - 1; j >= 0 && change - max_gain <= 0; j--)
    {
        int v = O[j];
        change -= W[u][v];
        change += W[v][u];
        max_gain -= W[u][v];
        if (change < 0)
        {
            last_pos = j;
            break;
        }
        if (change == 0)
            last_pos = j;
    }
    if (last_pos != i)
    {
        if (change < 0)
            *c += change;
        for (int k = i; k > last_pos; k--)
        {
            O[k] = O[k - 1];
            lr[O[k]] -= W[u][O[k]];
            ll[O[k]] += W[O[k]][u];

            lr[u] += W[O[k]][u];
            ll[u] -= W[u][O[k]];
        }
        O[last_pos] = u;
    }
    return change < 0;
}

void heuristics_greedy_improvement(comp c, volatile sig_atomic_t *tle)
{
    int *lr = malloc(sizeof(int) * c.n);
    int *ll = malloc(sizeof(int) * c.n);
    int *V = malloc(sizeof(int) * c.n);
    for (int i = 0; i < c.n; i++)
        V[i] = i;
    shuffle(V, c.n);

    for (int i = 0; i < c.n; i++)
    {
        int u = c.S[i];
        lr[u] = 0;
        ll[u] = 0;
        for (int j = i + 1; j < c.n; j++)
        {
            int v = c.S[j];
            lr[u] += c.W[v][u];
        }
        for (int j = 0; j < i; j++)
        {
            int v = c.S[j];
            ll[u] += c.W[u][v];
        }
    }

    int found = 1;
    while (found && !(*tle))
    {
        found = 0;
        for (int _i = 0; _i < c.n; _i++)
        {
            int i = V[_i];
            if (_i & 1)
            {
                int imp = try_right(c.W, c.S, c.c, i, c.n, lr, ll);
                if (!imp)
                    imp = try_left(c.W, c.S, c.c, i, c.n, lr, ll);
                found |= imp;
            }
            else
            {
                int imp = try_left(c.W, c.S, c.c, i, c.n, lr, ll);
                if (!imp)
                    imp = try_right(c.W, c.S, c.c, i, c.n, lr, ll);
                found |= imp;
            }
        }
    }

    free(lr);
    free(ll);
    free(V);
}

void heuristics_cut_matrix(int **cut_matrix, comp c)
{
    for (int i = 0; i < c.n; i++)
        cut_matrix[i][i] = 0;

    for (int i = 0; i < c.n; i++)
        for (int j = i + 1; j < c.n; j++)
            cut_matrix[i][j] = (c.W[c.S[i]][c.S[j]] - c.W[c.S[j]][c.S[i]]) + cut_matrix[i][j - 1];

    for (int j = 0; j < c.n; j++)
        for (int i = j - 1; i >= 0; i--)
            cut_matrix[i][j] += cut_matrix[i + 1][j];
}

void heuristics_swap_cut(comp c, int l, int k, int r, int *tmp, int change)
{
    *c.c += change;
    for (int i = 0; i < (k + 1) - l; i++)
        tmp[i] = c.S[l + i];

    for (int i = 0; i < r - k; i++)
        c.S[l + i] = c.S[(k + 1) + i];

    for (int i = 0; i < (k + 1) - l; i++)
        c.S[l + (r - k) + i] = tmp[i];
}

void heuristics_greedy_cut(comp c, volatile sig_atomic_t *tle)
{
    int *tmp = malloc(sizeof(int) * c.n);

    int *cut_data = malloc(sizeof(int) * c.n * c.n);
    for (int i = 0; i < c.n * c.n; i++)
        cut_data[i] = 0;

    int **cut_matrix = malloc(sizeof(int *) * c.n);
    for (int i = 0; i < c.n; i++)
        cut_matrix[i] = cut_data + i * c.n;

    heuristics_cut_matrix(cut_matrix, c);

    for (int i = 0; i < c.n && !(*tle); i++)
    {
        for (int j = i + 1; j < c.n; j++)
        {
            for (int k = i; k < j && k < i + 4; k++)
            {
                int d = cut_matrix[i][j] - cut_matrix[i][k] - cut_matrix[k + 1][j];
                if (d < 0)
                {
                    heuristics_swap_cut(c, i, k, j, tmp, d);
                    heuristics_cut_matrix(cut_matrix, c);
                }
            }
            for (int k = j - 4; k < j && k >= i + 4; k++)
            {
                int d = cut_matrix[i][j] - cut_matrix[i][k] - cut_matrix[k + 1][j];
                if (d < 0)
                {
                    heuristics_swap_cut(c, i, k, j, tmp, d);
                    heuristics_cut_matrix(cut_matrix, c);
                }
            }
        }
    }

    free(tmp);
    free(cut_data);
    free(cut_matrix);
}

void heuristic_randomize_solution(comp c, int changes)
{
    for (int i = 0; i < changes; i++)
    {
        int s = rand() % c.n;
        int t = rand() % c.n;

        int u = c.S[s];
        c.S[s] = c.S[t];
        c.S[t] = u;
    }

    *c.c = 0;
    for (int i = 0; i < c.n; i++)
        for (int j = i + 1; j < c.n; j++)
            *c.c += c.W[c.S[j]][c.S[i]];
}