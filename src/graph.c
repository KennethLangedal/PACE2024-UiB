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

static inline int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

graph parse_graph(FILE *f)
{
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = mmap(0, size, PROT_READ, MAP_PRIVATE, fileno_unlocked(f), 0);
    size_t i = 0;

    while (data[i] == 'c')
        skip_line(data, &i);

    graph g;
    g.old_label = NULL;

    int M;

    parse_id(data, &i, &g.A);
    parse_id(data, &i, &g.B);
    parse_id(data, &i, &M);

    g.N = g.A + g.B;

    if (data[i] != '\n' && data[i] != 13)
    {
        for (int j = 0; j < g.N; j++)
            skip_line(data, &i);
    }

    int *I = malloc(sizeof(int) * M);
    int *J = malloc(sizeof(int) * M);

    g.V = malloc(sizeof(int) * (g.N + 1));
    for (int j = 0; j < g.N + 1; j++)
        g.V[j] = 0;

    for (int j = 0; j < M; j++)
    {
        skip_line(data, &i);

        while (data[i] == 'c')
            skip_line(data, &i);

        parse_id(data, &i, I + j);
        parse_id(data, &i, J + j);

        I[j]--;
        J[j]--;

        g.V[I[j]]++;
        g.V[J[j]]++;
    }

    for (int j = 0; j < g.N; j++)
        g.V[j + 1] += g.V[j];

    g.E = malloc(sizeof(int) * g.V[g.N]);

    for (int j = 0; j < M; j++)
    {
        g.V[I[j]]--;
        g.E[g.V[I[j]]] = J[j];

        g.V[J[j]]--;
        g.E[g.V[J[j]]] = I[j];
    }

    for (int j = 0; j < g.N; j++)
        qsort(g.E + g.V[j], g.V[j + 1] - g.V[j], sizeof(int), compare);

    free(I);
    free(J);

    munmap(data, size);

    g.twins = malloc(sizeof(int) * g.N);
    g.old_label = malloc(sizeof(int) * g.N);

    for (int i = 0; i < g.N; i++)
    {
        g.twins[i] = 1;
        g.old_label[i] = i;
    }

    make_simple(g.N, g.V, &g.E);

    return g;
}

void store_graph(FILE *f, graph g)
{
    fprintf(f, "p ocr %d %d %d\n", g.A, g.B, g.V[g.N]);
    for (int u = 0; u < g.A; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            fprintf(f, "%d %d\n", u + 1, v + 1);
        }
    }
}

graph subgraph(graph g, int *mask)
{
    graph sg = {.N = 0, .A = 0, .B = 0};

    int *new_label = malloc(sizeof(int) * g.N);
    int M = 0;

    for (int i = 0; i < g.N; i++)
    {
        if (!mask[i])
            continue;

        new_label[i] = sg.N++;
        if (i < g.A)
            sg.A++;
        else
            sg.B++;

        for (int j = g.V[i]; j < g.V[i + 1]; j++)
            if (mask[g.E[j]])
                M++;
    }

    sg.V = malloc(sizeof(int) * (sg.N + 1));
    sg.E = malloc(sizeof(int) * M);
    sg.twins = malloc(sizeof(int) * sg.N);
    sg.old_label = malloc(sizeof(int) * sg.N);

    sg.V[0] = 0;
    for (int i = 0; i < g.N; i++)
    {
        if (!mask[i])
            continue;

        int u = new_label[i];
        sg.twins[u] = g.twins[i];
        sg.old_label[u] = i;

        int degree = 0;
        for (int j = g.V[i]; j < g.V[i + 1]; j++)
        {
            if (!mask[g.E[j]])
                continue;

            sg.E[sg.V[u] + degree] = new_label[g.E[j]];
            degree++;
        }
        sg.V[u + 1] = sg.V[u] + degree;
    }

    free(new_label);
    return sg;
}

graph remove_degree_zero(graph g)
{
    int *mask = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        mask[i] = 0;

    for (int i = g.A; i < g.N; i++)
    {
        if (g.V[i + 1] - g.V[i] < 1)
            continue;

        mask[i] = 1;
        for (int j = g.V[i]; j < g.V[i + 1]; j++)
            mask[g.E[j]] = 1;
    }

    graph sg = subgraph(g, mask);

    free(mask);

    return sg;
}

graph remove_twins(graph g)
{
    int *mask = malloc(sizeof(int) * g.N);
    int *twins = malloc(sizeof(int) * g.N);

    for (int u = 0; u < g.N; u++)
    {
        mask[u] = 1;
        twins[u] = 1;
    }

    for (int u = g.A; u < g.N; u++)
    {
        if (!mask[u])
            continue;
        int degree = g.V[u + 1] - g.V[u];
        if (degree < 1)
            continue;
        int w = g.E[g.V[u]];

        for (int k = g.V[w]; k < g.V[w + 1]; k++)
        {
            int v = g.E[k];
            if (u == v || degree != g.V[v + 1] - g.V[v])
                continue;

            int twin = 1;
            int i = g.V[u], j = g.V[v];
            while (i < g.V[u + 1])
            {
                if (g.E[i] != g.E[j])
                {
                    twin = 0;
                    break;
                }
                i++;
                j++;
            }
            if (twin)
            {
                twins[u]++;
                mask[v] = 0;
            }
        }
    }

    graph sg = subgraph(g, mask);
    for (int u = 0; u < sg.N; u++)
        sg.twins[u] = twins[sg.old_label[u]];

    free(mask);
    free(twins);

    return sg;
}

graph *split_graph(graph g, int *N)
{
    int *l = malloc(sizeof(int) * g.A);
    int *r = malloc(sizeof(int) * g.A);

    for (int i = 0; i < g.A; i++)
    {
        l[i] = 0;
        r[i] = 0;
    }

    for (int i = g.A; i < g.N; i++)
    {
        if (g.V[i + 1] - g.V[i] <= 1)
            continue;

        l[g.E[g.V[i]]]++;
        r[g.E[g.V[i + 1] - 1]]++;
    }

    *N = 0;
    int prev = 0;
    int c = 0;
    for (int i = 0; i < g.A; i++)
    {
        c -= r[i];
        if (c == 0 && i > prev)
        {
            prev = i;
            (*N)++;
        }
        c += l[i];
    }

    graph *cc = malloc(sizeof(graph) * (*N));
    int *marks = malloc(sizeof(int) * g.N);

    *N = 0;
    prev = 0;
    c = 0;
    for (int i = 0; i < g.A; i++)
    {
        c -= r[i];
        if (c == 0 && i > prev)
        {
            for (int j = 0; j < g.N; j++)
                marks[j] = 0;
            for (int j = prev; j <= i; j++)
                marks[j] = 1;

            for (int j = g.A; j < g.N; j++)
            {
                if (g.V[j + 1] - g.V[j] == 1)
                {
                    if (g.E[g.V[j]] >= prev && (g.E[g.V[j]] < i || i == g.A - 1))
                        marks[j] = 1;
                }
                else if (g.E[g.V[j]] >= prev && g.E[g.V[j + 1] - 1] <= i)
                    marks[j] = 1;
            }
            cc[*N] = subgraph(g, marks);

            prev = i;
            (*N)++;
        }
        c += l[i];
    }

    free(l);
    free(r);
    free(marks);

    return cc;
}

int test_twin(graph g, int u, int v)
{
    if (g.V[u + 1] - g.V[u] != g.V[v + 1] - g.V[v])
        return 0;

    int twin = 1;
    int i = g.V[u], j = g.V[v];
    while (i < g.V[u + 1])
    {
        if (g.E[i] != g.E[j])
        {
            twin = 0;
            break;
        }
        i++;
        j++;
    }

    return twin;
}

void free_graph(graph g)
{
    free(g.V);
    free(g.E);
    free(g.twins);
    free(g.old_label);
}

int validate_graph(graph g)
{
    int M = 0;
    for (int u = 0; u < g.N; u++)
    {
        if (g.V[u + 1] - g.V[u] < 0)
            return 2;

        M += g.V[u + 1] - g.V[u];

        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            if (i < 0 || i >= g.V[g.N])
                return 3;

            int v = g.E[i];
            if (v < 0 || v >= g.N || v == u || (i > g.V[u] && v <= g.E[i - 1]))
                return 4;

            if (bsearch(&u, g.E + g.V[v], g.V[v + 1] - g.V[v], sizeof(int), compare) == NULL)
                return 5;

            if ((u < g.A && v < g.A) || (u >= g.A && v >= g.A))
            {
                printf("%d %d\n", u + 1, v + 1);
                return 6;
            }
        }
    }

    if (M != g.V[g.N])
        return 7;

    return 1;
}