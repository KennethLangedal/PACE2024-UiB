#include "dfas.h"

#include <stdlib.h>

static inline void count_crossings(int *V, int *E, int u, int v, int *uv, int *vu)
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

static inline int compare_ranges(const void *a, const void *b, void *arg)
{
    int u = *(int *)a, v = *(int *)b;
    ocm *p = (ocm *)arg;

    if (p->V[u + 1] - p->V[u] == 0)
        return -(p->V[v + 1] - p->V[v]);
    if (p->V[v + 1] - p->V[v] == 0)
        return p->V[u + 1] - p->V[u];

    int d1 = p->E[p->V[u]] - p->E[p->V[v]];
    if (d1 != 0)
        return d1;
    return p->E[p->V[u + 1] - 1] - p->E[p->V[v + 1] - 1];
}

typedef struct
{
    int u, v;
} edge;

static inline int compare_edges(const void *a, const void *b)
{
    edge *e1 = (edge *)a, *e2 = (edge *)b;
    if (e1->u != e2->u)
        return e1->u - e2->u;
    return e1->v - e2->v;
}

comp comp_init(int n, int *V, ocm p)
{
    if (n > (1 << 12))
        return (comp){.n = -1, .W = NULL, .c = NULL, .S = NULL, .I = NULL};

    comp cc = {.n = n};
    int *data = malloc(sizeof(int) * n * n);
    cc.W = malloc(sizeof(int *) * n);
    for (int i = 0; i < n; i++)
        cc.W[i] = data + i * n;

    cc.c = malloc(sizeof(int));
    cc.S = malloc(sizeof(int) * n);
    cc.I = malloc(sizeof(int) * n);

    for (int i = 0; i < n; i++)
        cc.S[i] = i;

    for (int i = 0; i < n; i++)
        cc.I[i] = V[i];

    *cc.c = 0;
    for (int i = 0; i < n; i++)
    {
        cc.W[i][i] = 0;
        int u = cc.I[i], v, uv, vu;
        for (int j = i + 1; j < n; j++)
        {
            v = cc.I[j];
            count_crossings(p.V, p.E, u, v, &uv, &vu);
            if (uv < vu)
            {
                cc.W[i][j] = vu - uv;
                cc.W[j][i] = 0;
            }
            else
            {
                *cc.c += uv - vu;
                cc.W[j][i] = uv - vu;
                cc.W[i][j] = 0;
            }
        }
    }
    return cc;
}

void dfas_free_comp(comp c)
{
    if (c.W != NULL)
        free(*c.W);
    free(c.W);
    free(c.c);
    free(c.S);
    free(c.I);
}

typedef struct
{
    int n;
    int *stack, *on_stack;
    int id;
    int *index, *lowlink;
    int *tmp;
} tarjan;

tarjan tarjan_init(int n)
{
    tarjan t = {.n = 0, .id = 0};
    t.stack = malloc(sizeof(int) * n);
    t.on_stack = malloc(sizeof(int) * n);
    t.index = malloc(sizeof(int) * n);
    t.lowlink = malloc(sizeof(int) * n);
    t.tmp = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
    {
        t.on_stack[i] = 0;
        t.index[i] = -1;
    }
    return t;
}

void tarjan_free(tarjan t)
{
    free(t.stack);
    free(t.on_stack);
    free(t.index);
    free(t.lowlink);
    free(t.tmp);
}

void tarjan_explore(int u, int *V, edge *E, tarjan *t, ocm p, comp *C, int *n)
{
    t->index[u] = t->id;
    t->lowlink[u] = t->id;
    t->id += 1;

    t->stack[t->n] = u;
    t->n += 1;
    t->on_stack[u] = 1;

    for (int i = V[u]; i < V[u + 1]; i++)
    {
        int v = E[i].v;
        if (t->index[v] < 0)
        {
            tarjan_explore(v, V, E, t, p, C, n);
            if (t->lowlink[v] < t->lowlink[u])
                t->lowlink[u] = t->lowlink[v];
        }
        else if (t->on_stack[v] && t->index[v] < t->lowlink[u])
        {
            t->lowlink[u] = t->index[v];
        }
    }

    if (t->lowlink[u] == t->index[u])
    {
        int v, c = 0;
        do
        {
            t->n -= 1;
            v = t->stack[t->n];
            t->on_stack[v] = 0;
            t->tmp[c++] = v;
        } while (v != u);
        C[(*n)++] = comp_init(c, t->tmp, p);
    }
}

dfas dfas_construct(ocm p)
{
    int max_edges = 200000000;

    int m = 0;
    edge *E = malloc(sizeof(edge) * (max_edges + 1));

    dfas g = {.n = 0, .offset = 0};
    g.O = malloc(sizeof(int) * p.n1);
    for (int i = 0; i < p.n1; i++)
        g.O[i] = i;

    qsort_r(g.O, p.n1, sizeof(int), compare_ranges, &p);

    for (int i = 0; i < p.n1; i++)
    {
        int u = g.O[i];
        if (p.V[u + 1] - p.V[u] <= 1)
            continue;

        for (int j = i + 1; j < p.n1; j++)
        {
            int v = g.O[j];
            if (p.E[p.V[u + 1] - 1] <= p.E[p.V[v]])
                break;

            int c_uv = 0, c_vu = 0;
            count_crossings(p.V, p.E, u, v, &c_uv, &c_vu);

            if (c_uv != c_vu)
            {
                if (m >= max_edges)
                {
                    free(E);
                    return (dfas){.n = -1, .C = NULL, .O = NULL, .offset = 0};
                }

                if (c_uv < c_vu)
                    E[m] = (edge){.u = u, .v = v};
                else
                    E[m] = (edge){.u = v, .v = u};
                m++;

                if (c_uv < c_vu)
                    g.offset += c_uv;
                else
                    g.offset += c_vu;
            }
            else
            {
                g.offset += c_uv;
            }
        }
    }

    qsort(E, m, sizeof(edge), compare_edges);
    E[m] = (edge){.u = p.n1, .v = p.n1};

    int *V = malloc(sizeof(int) * (p.n1 + 1));

    m = 0;
    V[0] = 0;
    for (int u = 0; u < p.n1; u++)
    {
        while (E[m].u == u)
            m++;
        V[u + 1] = m;
    }

    tarjan t = tarjan_init(p.n1);
    g.C = malloc(sizeof(comp) * p.n1);
    for (int i = p.n1 - 1; i >= 0; i--)
    {
        int u = g.O[i];
        if (t.index[u] < 0)
            tarjan_explore(u, V, E, &t, p, g.C, &g.n);
    }
    g.C = realloc(g.C, sizeof(comp) * g.n);

    free(E);
    free(V);
    tarjan_free(t);

    return g;
}

void dfas_free(dfas g)
{
    for (int i = 0; i < g.n; i++)
        dfas_free_comp(g.C[i]);
    free(g.C);
    free(g.O);
}

comp dfas_construct_subgraph(comp c, int *E, int m)
{
    int *new_id = malloc(sizeof(int) * c.n);
    for (int i = 0; i < c.n; i++)
        new_id[i] = -1;
    int n = 0;
    for (int i = 0; i < m; i++)
    {
        int u = E[i] / c.n;
        int v = E[i] % c.n;
        if (new_id[u] < 0)
            new_id[u] = n++;
        if (new_id[v] < 0)
            new_id[v] = n++;
    }

    int *data = malloc(sizeof(int) * n * n);
    int **W = malloc(sizeof(int *) * n);
    for (int i = 0; i < n; i++)
        W[i] = data + i * n;

    int *cr = malloc(sizeof(int));
    *cr = 0;
    int *S = malloc(sizeof(int) * n);
    int *I = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++)
        I[i] = -1;

    for (int i = 0; i < m; i++)
    {
        int u = E[i] / c.n;
        int v = E[i] % c.n;
        I[new_id[u]] = u;
        I[new_id[v]] = v;

        W[new_id[u]][new_id[v]] = c.W[u][v];
    }

    for (int i = 0; i < n; i++)
    {
        S[i] = i;
        for (int j = i + 1; j < n; j++)
            *cr += W[j][i];
    }

    free(new_id);

    return (comp){.n = n, .W = W, .c = cr, .S = S, .I = I};
}

int *dfas_get_solution(ocm p, dfas g)
{

    int *S = malloc(sizeof(int) * p.n1);
    int _i = p.n1;
    for (int i = 0; i < g.n; i++)
    {
        _i -= g.C[i].n;
        for (int j = 0; j < g.C[i].n; j++)
        {
            S[_i + j] = g.C[i].I[g.C[i].S[j]];
        }
    }
    return S;
}
