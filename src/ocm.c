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