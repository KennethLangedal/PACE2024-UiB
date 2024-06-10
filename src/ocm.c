#include "ocm.h"

#include <stdlib.h>

static inline void skip_comments(FILE *f)
{
    int c = fgetc_unlocked(f);
    while (c == 'c')
    {
        while (c != '\n')
            c = fgetc_unlocked(f);
        c = fgetc_unlocked(f);
    }
    ungetc(c, f);
}

static inline void skip_line(FILE *f)
{
    int c = fgetc_unlocked(f);
    while (c != '\n')
        c = fgetc_unlocked(f);
}

static inline void parse_unsigned_int(FILE *f, int *v)
{
    int c = fgetc_unlocked(f);
    while ((c < '0' || c > '9') && c != '\n')
        c = fgetc_unlocked(f);

    *v = -1;
    if (c == '\n')
    {
        ungetc(c, f);
        return;
    }

    *v = 0;
    while (c >= '0' && c <= '9')
    {
        *v = (*v * 10) + (c - '0');
        c = fgetc_unlocked(f);
    }
    ungetc(c, f);
}

static inline int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

ocm ocm_parse(FILE *f)
{
    skip_comments(f);
    ocm p;
    parse_unsigned_int(f, &p.n0);
    parse_unsigned_int(f, &p.n1);
    parse_unsigned_int(f, &p.m);
    parse_unsigned_int(f, &p.cw);

    if (p.cw >= 0)
    {
        p.C = malloc(sizeof(int) * (p.n0 + p.n1));
        for (int i = 0; i < p.n0 + p.n1; i++)
        {
            skip_line(f);
            skip_comments(f);
            parse_unsigned_int(f, p.C + i);
            p.C[i]--;
        }
    }
    else
    {
        p.C = NULL;
    }

    int *X = malloc(sizeof(int) * p.m);
    int *Y = malloc(sizeof(int) * p.m);

    int *D = malloc(sizeof(int) * p.n1);
    for (int i = 0; i < p.n1; i++)
        D[i] = 0;

    for (int i = 0; i < p.m; i++)
    {
        skip_line(f);
        skip_comments(f);
        parse_unsigned_int(f, X + i);
        parse_unsigned_int(f, Y + i);
        X[i]--;
        Y[i]--;
        D[Y[i] - p.n0]++;
    }

    p.V = malloc(sizeof(int) * (p.n1 + 1));
    p.E = malloc(sizeof(int) * p.m);

    p.V[0] = 0;
    for (int i = 0; i < p.n1; i++)
        p.V[i + 1] = p.V[i] + D[i];

    for (int i = 0; i < p.n1; i++)
        D[i] = 0;

    for (int i = 0; i < p.m; i++)
    {
        int u = X[i], v = Y[i] - p.n0;
        p.E[p.V[v] + D[v]] = u;
        D[v]++;
    }

    for (int i = 0; i < p.n1; i++)
        qsort(p.E + p.V[i], p.V[i + 1] - p.V[i], sizeof(int), compare);

    free(X);
    free(Y);
    free(D);

    return p;
}

void ocm_free(ocm p)
{
    free(p.V);
    free(p.E);
    free(p.C);
}

int ocm_validate(ocm p)
{
    for (int u = 0; u < p.n1; u++)
    {
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            int v = p.E[i];
            if (i > p.V[u] && v <= p.E[i - 1])
                return 0;
            if (v < 0 || v >= p.n0)
                return 0;
        }
    }
    return 1;
}

static inline void ocm_count_crossings(int *V, int *E, int u, int v, int *uv, int *vu)
{
    *uv = 0;
    *vu = 0;
    int i = V[u], j = V[v];
    while (i < V[u + 1] && j < V[v + 1])
    {
        if (E[i] < E[j])
        {
            *vu += V[v + 1] - j;
            i++;
        }
        else if (E[i] > E[j])
        {
            *uv += V[u + 1] - i;
            j++;
        }
        else
        {
            i++;
            j++;
            *uv += V[u + 1] - i;
            *vu += V[v + 1] - j;
        }
    }
}

static inline int compare_avg(const void *a, const void *b, void *arg)
{
    int u = *(int *)a, v = *(int *)b;
    float *avg = (float *)arg;

    if (avg[u] < avg[v])
        return -1;
    if (avg[v] < avg[u])
        return 1;
    return 0;
}

int *ocm_average_placement(ocm p)
{
    float *avg = malloc(sizeof(float) * p.n1);
    for (int u = 0; u < p.n1; u++)
    {
        avg[u] = 0.0f;
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
            avg[u] += p.E[i];

        avg[u] /= (float)(p.V[u + 1] - p.V[u]);
    }

    int *S = malloc(sizeof(int) * p.n1);
    for (int u = 0; u < p.n1; u++)
        S[u] = u;

    qsort_r(S, p.n1, sizeof(int), compare_avg, avg);

    free(avg);
    return S;
}

const int ABORT_LIMIT = 10000;

int ocm_try_right(ocm p, int *O, int *c, int i)
{
    int u = O[i];
    int change = 0, best_change = 0;
    int last_pos = i;
    for (int j = i + 1; j < p.n1 && change < ABORT_LIMIT; j++)
    {
        int v = O[j];

        int uv, vu;
        ocm_count_crossings(p.V, p.E, u, v, &uv, &vu);
        change -= uv;
        change += vu;
        if (change < best_change)
        {
            best_change = change;
            last_pos = j;
            // break;
        }
        else if (change == best_change)
            last_pos = j;
    }
    if (last_pos != i)
    {
        if (best_change < 0)
            *c += best_change;
        for (int k = i; k < last_pos; k++)
            O[k] = O[k + 1];
        O[last_pos] = u;
    }
    return best_change < 0;
}

int ocm_try_left(ocm p, int *O, int *c, int i)
{
    int u = O[i];
    int change = 0, best_change = 0;
    int last_pos = i;
    for (int j = i - 1; j >= 0 && change < ABORT_LIMIT; j--)
    {
        int v = O[j];

        int uv, vu;
        ocm_count_crossings(p.V, p.E, u, v, &uv, &vu);
        change -= vu;
        change += uv;
        if (change < best_change)
        {
            best_change = change;
            last_pos = j;
            // break;
        }
        else if (change == best_change)
            last_pos = j;
    }
    if (last_pos != i)
    {
        if (best_change < 0)
            *c += best_change;
        for (int k = i; k > last_pos; k--)
            O[k] = O[k - 1];
        O[last_pos] = u;
    }
    return best_change < 0;
}

void ocm_shuffle(int *array, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

void ocm_greedy_improvement(ocm p, int *S, int *c, volatile sig_atomic_t *tle)
{
    int *O = malloc(sizeof(int) * p.n1);
    for (int i = 0; i < p.n1; i++)
        O[i] = i;
    ocm_shuffle(O, p.n1);

    int found = 1;
    while (found && !(*tle))
    {
        found = 0;
        for (int _i = 0; _i < p.n1 && !(*tle); _i++)
        {
            int i = O[_i];
            if (_i & 1)
            {
                int imp = ocm_try_right(p, S, c, i);
                if (!imp)
                    imp = ocm_try_left(p, S, c, i);
                found |= imp;
            }
            else
            {
                int imp = ocm_try_left(p, S, c, i);
                if (!imp)
                    imp = ocm_try_right(p, S, c, i);
                found |= imp;
            }
        }
        // fprintf(stderr, "\r%d", *c);
        // fflush(stderr);
    }

    free(O);
}