#include "cycle_packing.h"

#include <stdlib.h>

void add_cycle(packing p, int *E, int n)
{
    int min_C = p.C[E[0]];
    for (int i = 1; i < n; i++)
        if (p.C[E[i]] < min_C)
            min_C = p.C[E[i]];

    if (min_C == 0)
        return;

    cycle *c = malloc(sizeof(cycle));
    c->n = n;
    c->w = min_C;
    c->e = malloc(sizeof(int) * n);
    c->p = malloc(sizeof(int) * n);

    for (int i = 0; i < n; i++)
    {
        int e = E[i];
        int pe = p.V[e];
        while (p.edges[pe] != NULL)
            pe++;

        c->e[i] = e;
        c->p[i] = pe;

        p.edges[pe] = c;
        p.C[e] -= c->w;
    }

    *p.c += c->w;
}

void remove_cycle(packing p, cycle *c)
{
    for (int i = 0; i < c->n; i++)
    {
        int e = c->e[i];
        int pe = c->p[i];
        p.edges[pe] = NULL;
        p.C[e] += c->w;
    }

    *p.c -= c->w;

    free(c->e);
    free(c->p);
    free(c);
}

packing cycle_packing_init(int **W, int n)
{
    int sum = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            sum += W[i][j];

    packing p = {.n = n * n, ._n = n, .m = sum};
    p.V = malloc(sizeof(int) * (p.n + 1));
    p.C = malloc(sizeof(int) * (p.n));
    p.c = malloc(sizeof(int));
    p.edges = malloc(sizeof(cycle *) * p.m);

    for (int i = 0; i < p.m; i++)
        p.edges[i] = NULL;

    *p.c = 0;
    p.V[0] = 0;
    for (int i = 0; i < p.n; i++)
    {
        p.V[i + 1] = p.V[i] + (*W)[i];
        p.C[i] = (*W)[i];
    }

    return p;
}

void cycle_packing_free(packing p)
{
    for (int i = 0; i < p.m; i++)
        if (p.edges[i] != NULL)
            remove_cycle(p, p.edges[i]);

    free(p.V);
    free(p.C);
    free(p.c);
    free(p.edges);
}

void cycle_packing_greedy_3(packing p, int *order)
{
    int *E = malloc(sizeof(int) * 3);
    for (int _u = 0; _u < p._n; _u++)
    {
        int u = order[_u];
        for (int _v = _u + 1; _v < p._n; _v++)
        {
            int v = order[_v];
            int uv = u * p._n + v;
            E[0] = uv;
            for (int _w = _u + 1; _w < p._n && p.C[uv] > 0; _w++)
            {
                int w = order[_w];
                int vw = v * p._n + w;
                int wu = w * p._n + u;
                if (w != v && p.C[vw] > 0 && p.C[wu] > 0)
                {
                    E[1] = vw;
                    E[2] = wu;
                    add_cycle(p, E, 3);
                }
            }
        }
    }
    free(E);
}

void cycle_packing_replacing_3_fast(packing p, int *order)
{
    int *E = malloc(sizeof(int) * p._n);
    for (int _u = 0; _u < p._n; _u++)
    {
        int u = order[_u];
        for (int _v = 0; _v < p._n; _v++)
        {
            int v = order[_v];
            int uv = u * p._n + v;
            for (int pe = p.V[uv]; pe < p.V[uv + 1]; pe++)
            {
                cycle *c = p.edges[pe];
                if (c == NULL || c->e[0] != uv)
                    continue;

                int n = 0;
                for (int i = 0; i < c->n; i++)
                {
                    E[i] = -1;
                    int e = c->e[i];
                    int x = e / p._n, y = e % p._n;
                    for (int z = 0; z < p._n; z++)
                    {
                        if (p.C[y * p._n + z] > 0 && p.C[z * p._n + x] > 0)
                        {
                            E[i] = z;
                            n++;
                            break;
                        }
                    }
                }
                if (n > 1)
                {
                    int m = c->n;
                    int *ce = malloc(sizeof(int) * c->n);
                    for (int i = 0; i < c->n; i++)
                        ce[i] = c->e[i];

                    int alt_c[3];
                    remove_cycle(p, c);
                    for (int i = 0; i < m; i++)
                    {
                        if (E[i] < 0)
                            continue;

                        int e = ce[i];
                        int x = e / p._n, y = e % p._n, z = E[i];
                        alt_c[0] = e;
                        alt_c[1] = y * p._n + z;
                        alt_c[2] = z * p._n + x;
                        add_cycle(p, alt_c, 3);
                    }
                    free(ce);
                }
            }
        }
    }
    free(E);
}

void cycle_packing_replacing_3(packing p, int *order)
{
    int *E = malloc(sizeof(int) * p._n);
    for (int _u = 0; _u < p._n; _u++)
    {
        int u = order[_u];
        for (int _v = 0; _v < p._n; _v++)
        {
            int v = order[_v];
            int uv = u * p._n + v;
            if (u == v || p.V[uv + 1] - p.V[uv] == 0 || p.C[uv] > 0)
                continue;

            int pe = -1;
            for (int i = p.V[uv]; i < p.V[uv + 1]; i++)
                if (p.edges[i] != NULL && (pe < 0 || p.edges[i]->w < p.edges[pe]->w))
                    pe = i;

            cycle *c = p.edges[pe];
            // Fully used edge

            int uw = -1, wv = -1;
            for (int w = 0; w < p._n; w++)
            {
                int _uw = u * p._n + w;
                int _wv = w * p._n + v;
                if (w != u && w != v && p.C[_uw] >= c->w && p.C[_wv] >= c->w)
                {
                    uw = _uw;
                    wv = _wv;
                    break;
                }
            }
            if (uw < 0)
                continue;

            int vw = -1, wu = -1;
            for (int w = 0; w < p._n; w++)
            {
                int _vw = v * p._n + w;
                int _wu = w * p._n + u;
                if (w != u && w != v && p.C[_vw] >= c->w && p.C[_wu] >= c->w)
                {
                    vw = _vw;
                    wu = _wu;
                    break;
                }
            }

            if (vw >= 0)
            {
                int n = 0;
                for (int i = 0; i < c->n; i++)
                    if (c->e[i] != uv)
                        E[n++] = c->e[i];
                remove_cycle(p, c);
                E[n++] = uw;
                E[n++] = wv;
                add_cycle(p, E, n);

                E[0] = uv;
                E[1] = vw;
                E[2] = wu;
                add_cycle(p, E, 3);
            }
        }
    }
    free(E);
}

int packing_explore(packing p, int s, int t, int *E, int min, int max_d)
{
    int *prev = malloc(sizeof(int) * p._n);
    int *current = malloc(sizeof(int) * p._n);
    int *next = malloc(sizeof(int) * p._n);

    for (int i = 0; i < p._n; i++)
        prev[i] = -1;

    int n = 0, m = 0, found = 0;
    prev[s] = p._n;
    for (int i = 0; i < p._n; i++)
    {
        if (p.C[s * p._n + i] >= min)
        {
            prev[i] = s;
            current[n++] = i;
        }
    }

    int d = 0;
    while (n > 0)
    {
        if (++d >= max_d)
            break;
        m = 0;
        for (int i = 0; i < n; i++)
        {
            int u = current[i];
            for (int j = 0; j < p._n; j++)
            {
                if (prev[j] < 0 && p.C[u * p._n + j] >= min)
                {
                    if (j == t)
                    {
                        int v = t;
                        int e = 0;
                        do
                        {
                            E[e++] = u * p._n + v;
                            v = u;
                            u = prev[u];
                        } while (v != s);

                        free(prev);
                        free(current);
                        free(next);
                        return e;
                    }
                    prev[j] = u;
                    next[m++] = j;
                }
            }
        }
        n = m;
        int *t = current;
        current = next;
        next = t;
    }

    free(prev);
    free(current);
    free(next);

    return 0;
}

int cycle_packing_add_long(packing p, int *order, int max)
{
    int *E = malloc(sizeof(int) * p._n);
    for (int _u = 0; _u < p._n; _u++)
    {
        int u = order[_u];
        for (int _v = _u + 1; _v < p._n; _v++)
        {
            int v = order[_v];
            int uv = u * p._n + v;
            if (u != v && p.C[uv] > 0)
            {
                int nc = packing_explore(p, v, u, E, 1, max);
                if (nc > 0 && nc <= max)
                {
                    E[nc++] = u * p._n + v;
                    add_cycle(p, E, nc);
                    // fprintf(stderr, "(%d) %d\n", nc, *p.c);
                }
            }
        }
    }
    free(E);
}

void shuffle_order(int *array, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

void cycle_packing(packing p)
{
    int *order = malloc(sizeof(int) * p._n);
    for (int i = 0; i < p._n; i++)
        order[i] = i;

    shuffle_order(order, p._n);
    int old_c = *p.c - 1;
    while (old_c < *p.c)
    {
        old_c = *p.c;
        cycle_packing_greedy_3(p, order);
        cycle_packing_replacing_3(p, order);
        cycle_packing_replacing_3_fast(p, order);
    }

    cycle_packing_add_long(p, order, 9999);

    free(order);
}
