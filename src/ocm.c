#include "ocm.h"

#include <stdlib.h>
#include <assert.h>

ocm init_ocm_problem(graph g)
{
    ocm p = {.crossings = 0, .lb = 0, .undecided = 0, .equal = 0, .N = g.B};

    int64_t *data = malloc(sizeof(int64_t) * p.N * p.N);
    p.cm = malloc(sizeof(int64_t *) * p.N);

    for (int i = 0; i < p.N; i++)
        p.cm[i] = data + i * p.N;

    for (int u = 0; u < p.N; u++)
        for (int v = 0; v < p.N; v++)
            p.cm[u][v] = count_crossings_pair(g, u + g.A, v + g.A);

    for (int u = 0; u < p.N; u++)
    {
        for (int v = u + 1; v < p.N; v++)
        {
            if (p.cm[u][v] == p.cm[v][u])
                p.equal++;
            else
                p.undecided++;
        }
    }

    int aligned_size = ((p.N + 31) / 32) * 32;
    char *tc_data = aligned_alloc(32, sizeof(char) * aligned_size * p.N);
    p.tc = malloc(sizeof(char *) * p.N);
    for (int i = 0; i < p.N; i++)
        p.tc[i] = tc_data + i * aligned_size;

    for (int i = 0; i < aligned_size * p.N; i++)
        tc_data[i] = 0;

    for (int i = 0; i < p.N; i++)
        p.tc[i][i] = 1;

    for (int i = 0; i < p.N; i++)
        for (int j = i + 1; j < p.N; j++)
            p.lb += p.cm[i][j] < p.cm[j][i] ? p.cm[i][j] : p.cm[j][i];

    return p;
}

ocm copy_ocm_problem(ocm s)
{
    ocm p = {.crossings = s.crossings,
             .lb = s.lb,
             .undecided = s.undecided,
             .equal = s.equal,
             .N = s.N};

    int64_t *data = malloc(sizeof(int64_t) * p.N * p.N);
    p.cm = malloc(sizeof(int64_t *) * p.N);

    for (int i = 0; i < p.N; i++)
        p.cm[i] = data + i * p.N;

    for (int i = 0; i < p.N * p.N; i++)
        p.cm[0][i] = s.cm[0][i];

    int aligned_size = ((p.N + 31) / 32) * 32;
    char *tc_data = malloc(sizeof(char) * aligned_size * p.N);
    p.tc = malloc(sizeof(char *) * p.N);
    for (int i = 0; i < p.N; i++)
        p.tc[i] = tc_data + i * p.N;

    for (int i = 0; i < aligned_size * p.N; i++)
        tc_data[i] = s.tc[0][i];

    return p;
}

void free_ocm_problem(ocm p)
{
    free(*p.cm);
    free(p.cm);
    free(*p.tc);
    free(p.tc);
}

void ocm_reduction_trivial(ocm *p)
{
    for (int i = 0; i < p->N; i++)
    {
        for (int j = i + 1; j < p->N; j++)
        {
            if (p->cm[i][j] == 0)
            {
                p->tc[i][j] = 1;
                if (p->cm[j][i] == 0)
                    p->equal--;
                else
                    p->undecided--;
            }
            else if (p->cm[j][i] == 0)
            {
                p->tc[j][i] = 1;
                p->undecided--;
            }
        }
    }
}

void ocm_reduction_2_1(graph g, ocm *p)
{
    for (int i = 0; i < p->N; i++)
    {
        for (int j = 0; j < p->N; j++)
        {
            if (p->cm[i][j] == 1 && p->cm[j][i] == 2 &&
                g.V[g.A + i + 1] - g.V[g.A + i] == 2 &&
                g.V[g.A + j + 1] - g.V[g.A + j] == 2 &&
                (g.E[g.V[g.A + i]] == g.E[g.V[g.A + j]] || g.E[g.V[g.A + i] + 1] == g.E[g.V[g.A + j] + 1]))
            {
                ocm_add_edge(p, i, j);
            }
        }
    }
}

void ocm_reduction_twins(graph g, ocm *p)
{
    for (int u = g.A; u < g.N; u++)
    {
        if (g.V[u + 1] - g.V[u] < 1)
            continue;

        int w = g.E[g.V[u]];
        for (int i = g.V[w]; i < g.V[w + 1]; i++)
        {
            int v = g.E[i];
            if (v <= u)
                continue;

            if (!p->tc[u - g.A][v - g.A] && !p->tc[v - g.A][u - g.A] && test_twin(g, u, v))
                ocm_add_edge(p, u - g.A, v - g.A);
        }
    }
}

void ocm_reduction_independent(ocm *p)
{
    if (p->N < 2)
        return;

    int found = 1;
    while (found)
    {
        found = 0;

        for (int i = 0; i < p->N; i++)
        {
            int v = -1;
            int c = 0;
            for (int j = 0; j < p->N; j++)
            {
                if (i == j || p->tc[i][j] || p->tc[j][i] || p->cm[i][j] == p->cm[j][i])
                    continue;

                v = j;
                c++;
            }

            if (c == 1)
            {
                if (p->cm[i][v] <= p->cm[v][i])
                    ocm_add_edge(p, i, v);
                else
                    ocm_add_edge(p, v, i);

                found = 1;
            }
        }
    }
}

void ocm_reduction_k_quick(ocm *p, int ub)
{
    int found = 1;
    while (found)
    {
        found = 0;
        for (int i = 0; i < p->N; i++)
        {
            for (int j = i + 1; j < p->N; j++)
            {
                if (p->tc[i][j] || p->tc[j][i])
                    continue;

                int u = i, v = j;
                if (p->cm[i][j] > p->cm[j][i])
                    u = j, v = i;

                if (p->crossings + p->lb + (p->cm[v][u] - p->cm[u][v]) >= ub)
                {
                    ocm_add_edge(p, u, v);
                    found = 1;
                }
            }
        }
    }
}

void ocm_reduction_k_full(ocm *p, int ub)
{
    int found = 1;
    while (found)
    {
        found = 0;
        for (int i = 0; i < p->N; i++)
        {
            for (int j = i + 1; j < p->N; j++)
            {
                if (p->tc[i][j] || p->tc[j][i])
                    continue;

                int u = i, v = j;
                if (p->cm[i][j] > p->cm[j][i])
                    u = j, v = i;

                int extra = ocm_try_edge(*p, v, u);

                if (p->crossings + p->lb + extra >= ub)
                {
                    ocm_add_edge(p, u, v);
                    found = 1;
                    continue;
                }

                extra = ocm_try_edge(*p, u, v);

                if (p->crossings + p->lb + extra >= ub)
                {
                    ocm_add_edge(p, v, u);
                    found = 1;
                }
            }
        }
    }
}

int ocm_try_edge(ocm p, int u, int v)
{
    int extra = 0;
    for (int i = 0; i < p.N; i++)
    {
        if (i != u && p.tc[i][u] == 0)
            continue;

        for (int j = 0; j < p.N; j++)
        {
            if (j == i || (j != v && p.tc[v][j] == 0) || p.tc[i][j] == 1)
                continue;

            extra += p.cm[i][j] > p.cm[j][i] ? p.cm[i][j] - p.cm[j][i] : 0;
        }
    }

    return extra;
}

void ocm_add_edge(ocm *p, int u, int v)
{
    for (int i = 0; i < p->N; i++)
    {
        if (i != u && p->tc[i][u] == 0)
            continue;

        for (int j = 0; j < p->N; j++)
        {
            if (j == i || (j != v && p->tc[v][j] == 0) || p->tc[i][j] == 1)
                continue;

            p->lb -= p->cm[i][j] > p->cm[j][i] ? p->cm[j][i] : p->cm[i][j];
            p->crossings += p->cm[i][j];
            p->tc[i][j] = 1;

            if (p->cm[i][j] == p->cm[j][i])
                p->equal++;
            else
                p->undecided--;
        }
    }
}

void ocm_add_edge_fast(ocm *p, int u, int v)
{
    for (int i = 0; i < p->N; i++)
    {
        if (i != u && p->tc[i][u] == 0)
            continue;

        for (int j = 0; j < p->N; j++)
        {
            p->tc[i][j] |= p->tc[v][j];
        }
    }
}

#include <immintrin.h>

void ocm_add_edge_avx(ocm *p, int u, int v)
{
    for (int i = 0; i < p->N; i++)
    {
        if (i != u && p->tc[i][u] == 0)
            continue;

        for (int j = 0; j < p->N; j += 32)
        {
            __m256i _i = _mm256_load_si256((const __m256i *)(p->tc[i] + j));
            __m256i _v = _mm256_load_si256((const __m256i *)(p->tc[v] + j));
            _i = _mm256_or_si256(_i, _v);
            _mm256_store_si256((void *)(p->tc[i] + j), _i);
        }
    }
}

void ocm_update(ocm *p)
{
    p->equal = 0;
    p->undecided = 0;
    p->lb = 0;
    p->crossings = 0;
    for (int i = 0; i < p->N; i++)
    {
        for (int j = i + 1; j < p->N; j++)
        {
            if (p->tc[i][j] || p->tc[j][i])
            {
                if (p->tc[i][j])
                    p->crossings += p->cm[i][j];
                else
                    p->crossings += p->cm[j][i];
            }
            else if (p->cm[i][j] == p->cm[j][i])
            {
                p->equal++;
                p->lb += p->cm[i][j];
            }
            else
            {
                p->undecided++;
                p->lb += p->cm[i][j] < p->cm[j][i] ? p->cm[i][j] : p->cm[j][i];
            }
        }
    }
}

int64_t count_crossings_pair(graph g, int u, int v)
{
    if (u == v)
        return 0;

    int64_t crossings = 0;

    int i = g.V[u];
    int j = g.V[v];

    while (i < g.V[u + 1] && j < g.V[v + 1])
    {
        if (g.E[j] < g.E[i])
        {
            crossings += g.V[u + 1] - i;
            j++;
        }
        else
            i++;
    }

    return crossings * g.twins[u] * g.twins[v];
}

int64_t count_crossings_solution(graph g, int *s)
{
    int64_t crossings = 0;

    for (int i = 0; i < g.B; i++)
        for (int j = i + 1; j < g.B; j++)
            crossings += count_crossings_pair(g, s[i], s[j]);

    return crossings;
}

void lift_solution_degree_zero(graph g, graph r, int **s)
{
    int *sl = malloc(sizeof(int) * g.B);

    int p = 0;
    for (int i = 0; i < r.B; i++)
        sl[p++] = r.old_label[(*s)[i]];

    for (int u = g.A; u < g.N; u++)
        if (g.V[u + 1] - g.V[u] < 1)
            sl[p++] = u;

    free(*s);
    *s = sl;
}

void lift_solution_twins(graph g, graph r, int **s)
{
    int *sl = malloc(sizeof(int) * g.B);

    int p = 0;
    for (int i = 0; i < r.B; i++)
    {
        int u = (*s)[i];
        int _u = r.old_label[u];
        sl[p++] = _u;
        if (r.twins[u] > 1)
        {
            int _w = g.E[g.V[_u]];
            for (int j = g.V[_w]; j < g.V[_w + 1]; j++)
            {
                int _v = g.E[j];
                if (_u != _v && test_twin(g, _u, _v))
                    sl[p++] = _v;
            }
        }
    }

    free(*s);
    *s = sl;
}

int count_relevant_vertices(ocm p)
{
    int decided = 0;
    for (int i = 0; i < p.N; i++)
    {
        int found = 0;
        for (int j = 0; j < p.N; j++)
        {
            if (i != j && !p.tc[i][j] && !p.tc[j][i] && p.cm[i][j] != p.cm[j][i])
            {
                found = 1;
                break;
            }
        }
        if (!found)
            decided++;
    }
    return p.N - decided;
}

void ocm_copy_tc(ocm s, ocm *d)
{
    for (int i = 0; i < s.N * s.N; i++)
        (*d->tc)[i] = (*s.tc)[i];

    d->crossings = s.crossings;
    d->lb = s.lb;
    d->undecided = s.undecided;
    d->equal = s.equal;
}

void ocm_copy_full(ocm s, ocm *d)
{
    ocm_copy_tc(s, d);
    d->N = s.N;
    for (int i = 0; i < s.N * s.N; i++)
        (*d->cm)[i] = (*s.cm)[i];
}