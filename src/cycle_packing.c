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

packing cycle_packing_init(comp c)
{
    int sum = 0;
    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            sum += c.W[i][j];

    packing p = {.n = c.n * c.n, ._n = c.n, .m = sum};
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
        p.V[i + 1] = p.V[i] + (*c.W)[i];
        p.C[i] = (*c.W)[i];
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

void cycle_packing_greedy_3(packing p)
{
    int *E = malloc(sizeof(int) * 3);
    for (int u = 0; u < p._n; u++)
    {
        for (int v = u + 1; v < p._n; v++)
        {
            int uv = u * p._n + v;
            E[0] = uv;
            for (int w = u + 1; w < p._n && p.C[uv] > 0; w++)
            {
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

void cycle_packing_greedy_4(packing p)
{
    int *E = malloc(sizeof(int) * 4);
    for (int u = 0; u < p._n; u++)
    {
        for (int v = u + 1; v < p._n; v++)
        {
            int uv = u * p._n + v;
            E[0] = uv;
            for (int w = u + 1; w < p._n && p.C[uv] > 0; w++)
            {
                int vw = v * p._n + w;
                E[1] = vw;
                if (w == v)
                    continue;

                for (int x = u + 1; x < p._n && p.C[uv] > 0 && p.C[vw] > 0; x++)
                {
                    int wx = w * p._n + x;
                    int xu = x * p._n + u;
                    if (x != v && x != w && p.C[wx] > 0 && p.C[xu] > 0)
                    {
                        E[2] = wx;
                        E[3] = xu;
                        add_cycle(p, E, 4);
                        fprintf(stderr, "%d\n", *p.c);
                    }
                }
            }
        }
    }
    free(E);
}

void cycle_packing_replacing_3(packing p)
{
    int *E = malloc(sizeof(int) * 64);
    for (int u = 0; u < p._n; u++)
    {
        for (int v = 0; v < p._n; v++)
        {
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

            int vw = -1, wu = -1, wx = -1, xu = -1;
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
            if (vw < 0)
            {
                for (int w = 0; w < p._n; w++)
                {
                    int _vw = v * p._n + w;
                    if (w != u && w != v && p.C[_vw] >= c->w)
                    {
                        for (int x = 0; x < p._n; x++)
                        {
                            int _wx = w * p._n + x;
                            int _xu = x * p._n + u;
                            if (x != u && x != v && x != w && p.C[_wx] >= c->w && p.C[_xu] >= c->w)
                            {
                                vw = _vw;
                                wx = _wx;
                                xu = _xu;
                                break;
                            }
                        }
                        if (vw >= 0)
                            break;
                    }
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
                if (wu >= 0)
                {
                    E[0] = uv;
                    E[1] = vw;
                    E[2] = wu;
                    add_cycle(p, E, 3);
                }
                else if (xu >= 0)
                {
                    E[0] = uv;
                    E[1] = vw;
                    E[2] = wx;
                    E[3] = xu;
                    add_cycle(p, E, 4);
                }
            }
        }
    }
    free(E);
}

int packing_explore(packing p, int s, int t, int *E, int min)
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

    while (n > 0)
    {
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

int cycle_packing_replacing_long(packing p)
{
    int *E = malloc(sizeof(int) * 64);
    int *E2 = malloc(sizeof(int) * 64);
    for (int u = 0; u < p._n; u++)
    {
        for (int v = 0; v < p._n; v++)
        {
            int uv = u * p._n + v;
            if (u != v && p.C[uv] > 0)
            {
                int nc = packing_explore(p, v, u, E, 1);
                if (nc > 0)
                {
                    E[nc++] = u * p._n + v;
                    add_cycle(p, E, nc);
                }
            }

            if (u == v || p.V[uv + 1] - p.V[uv] == 0 || p.C[uv] > 0)
                continue;

            int pe = -1;
            for (int i = p.V[uv]; i < p.V[uv + 1]; i++)
                if (p.edges[i] != NULL && (pe < 0 || p.edges[i]->w < p.edges[pe]->w))
                    pe = i;

            cycle *c = p.edges[pe];
            // Fully used edge

            int lbp = packing_explore(p, u, v, E, c->w);
            int lnew = packing_explore(p, v, u, E2, c->w);
            if (lbp > 0 && lnew > 0)
            {
                for (int i = 0; i < c->n; i++)
                    if (c->e[i] != uv)
                        E[lbp++] = c->e[i];
                remove_cycle(p, c);
                add_cycle(p, E, lbp);
                E2[lnew++] = u * p._n + v;
                add_cycle(p, E2, lnew);
            }
        }
    }
    free(E);
    free(E2);
}

void cycle_packing_move(packing p)
{
    int *E = malloc(sizeof(int) * 64);
    for (int u = 0; u < p._n; u++)
    {
        for (int v = 0; v < p._n; v++)
        {
            int uv = u * p._n + v;
            if (u == v || p.V[uv + 1] - p.V[uv] == 0 || p.C[uv] > 0)
                continue;

            int pe = -1;
            for (int i = p.V[uv]; i < p.V[uv + 1]; i++)
                if (p.edges[i] != NULL && (pe < 0 || p.edges[i]->w < p.edges[pe]->w))
                    pe = i;

            cycle *c = p.edges[pe];
            // Fully used edge

            int lbp = packing_explore(p, u, v, E, c->w);
            if (lbp > 0)
            {
                for (int i = 0; i < c->n; i++)
                    if (c->e[i] != uv)
                        E[lbp++] = c->e[i];
                remove_cycle(p, c);
                add_cycle(p, E, lbp);
            }
        }
    }
    free(E);
}

void cycle_packing_greedy(packing p)
{
    cycle_packing_greedy_3(p);
    int old_c = *p.c - 1;
    while (old_c < *p.c)
    {
        old_c = *p.c;
        cycle_packing_replacing_3(p);
        fprintf(stderr, "%d\n", *p.c);
    }

    cycle_packing_replacing_long(p);
    fprintf(stderr, "%d\n", *p.c);

    old_c = *p.c - 1;
    while (old_c < *p.c)
    {
        old_c = *p.c;
        cycle_packing_replacing_3(p);
        fprintf(stderr, "%d\n", *p.c);
    }

    cycle_packing_replacing_long(p);
    fprintf(stderr, "%d\n", *p.c);

    int ec = 0;
    for (int i = 0; i < p.n; i++)
        if (p.C[i] > 0)
            ec++;

    fprintf(stderr, "%d\n", ec);
}