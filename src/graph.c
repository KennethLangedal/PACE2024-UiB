#include "graph.h"

#include <stdlib.h>
#include <sys/mman.h>

void make_simple(int N, int *V, int **E)
{
    int i = 0, r = 0;
    for (int u = 0; u < N; u++)
    {
        int _r = r;
        for (int j = V[u]; j < V[u + 1]; j++)
        {
            int v = (*E)[j];
            if (v == u || (j > V[u] && v <= (*E)[j - 1]))
                r++;
            else
                (*E)[i++] = v;
        }
        V[u] -= _r;
    }
    V[N] -= r;

    if (r > 0)
        *E = realloc(*E, sizeof(int) * V[N]);
}

static inline void parse_id(char *data, size_t *p, int *v)
{
    while (data[*p] < '0' || data[*p] > '9')
        (*p)++;

    *v = 0;
    while (data[*p] >= '0' && data[*p] <= '9')
        *v = (*v) * 10 + data[(*p)++] - '0';
}

static inline void skip_line(char *data, size_t *i)
{
    while (data[*i] != '\n')
        (*i)++;
    (*i)++;
}

int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

void graph_parse(FILE *f, int *N, int *N0, int *N1, int **V, int **E)
{
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = mmap(0, size, PROT_READ, MAP_PRIVATE, fileno_unlocked(f), 0);
    size_t i = 0;

    while (data[i] == 'c')
        skip_line(data, &i);

    int M;

    parse_id(data, &i, N0);
    parse_id(data, &i, N1);
    parse_id(data, &i, &M);

    *N = (*N0) + (*N1);

    int *I = malloc(sizeof(int) * M);
    int *J = malloc(sizeof(int) * M);

    *V = realloc(*V, sizeof(int) * ((*N) + 1));
    for (int j = 0; j < (*N) + 1; j++)
        (*V)[j] = 0;

    for (int j = 0; j < M; j++)
    {
        skip_line(data, &i);

        while (data[i] == 'c')
            skip_line(data, &i);

        parse_id(data, &i, I + j);
        parse_id(data, &i, J + j);

        I[j]--;
        J[j]--;

        (*V)[I[j]]++;
        (*V)[J[j]]++;
    }

    for (int j = 0; j < (*N); j++)
        (*V)[j + 1] += (*V)[j];

    *E = realloc(*E, sizeof(int) * (*V)[*N]);

    for (int j = 0; j < M; j++)
    {
        (*V)[I[j]]--;
        (*E)[(*V)[I[j]]] = J[j];

        (*V)[J[j]]--;
        (*E)[(*V)[J[j]]] = I[j];
    }

    for (int j = 0; j < (*N); j++)
        qsort((*E) + (*V)[j], (*V)[j + 1] - (*V)[j], sizeof(int), compare);

    free(I);
    free(J);

    munmap(data, size);

    make_simple(*N, *V, E);
}

int graph_validate(int N, int N0, int N1, const int *V, const int *E)
{
    int M = 0;
    for (int u = 0; u < N; u++)
    {
        if (V[u + 1] - V[u] < 0)
            return 0;

        M += V[u + 1] - V[u];

        for (int i = V[u]; i < V[u + 1]; i++)
        {
            if (i < 0 || i >= V[N])
                return 0;

            int v = E[i];
            if (v < 0 || v >= N || v == u || (i > V[u] && v <= E[i - 1]))
                return 0;

            if (bsearch(&u, E + V[v], V[v + 1] - V[v], sizeof(int), compare) == NULL)
                return 0;

            if ((u < N0 && v < N0) || (u >= N0 && v >= N0))
                return 0;
        }
    }

    if (M != V[N])
        return 0;

    return 1;
}