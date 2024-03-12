#include "dfas.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

static inline int compare(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

dfas dfas_construct_instance(ocm p)
{
    dfas g = {.N = p.N};
    g.V = malloc(sizeof(int) * (p.N + 1));
    g.E = malloc(sizeof(int) * p.undecided);
    g.W = malloc(sizeof(int) * p.undecided);
    g.a = malloc(sizeof(int) * p.undecided);
    g.cc = malloc(sizeof(int) * p.N);
    g.cc_size = malloc(sizeof(int) * p.N);
    g.n_cc = malloc(sizeof(int));

    for (int i = 0; i <= p.N; i++)
        g.V[i] = 0;

    for (int i = 0; i < p.undecided; i++)
        g.a[i] = 1;

    for (int i = 0; i < p.N; i++)
    {
        for (int j = i + 1; j < p.N; j++)
        {
            if (p.tc[i][j] || p.tc[j][i] || p.cm[i][j] == p.cm[j][i])
                continue;

            if (p.cm[i][j] < p.cm[j][i])
                g.V[i]++;
            else
                g.V[j]++;
        }
    }

    for (int i = 1; i <= p.N; i++)
        g.V[i] += g.V[i - 1];

    for (int i = 0; i < p.N; i++)
    {
        for (int j = i + 1; j < p.N; j++)
        {
            if (p.tc[i][j] || p.tc[j][i] || p.cm[i][j] == p.cm[j][i])
                continue;

            if (p.cm[i][j] < p.cm[j][i])
            {
                g.V[i]--;
                g.E[g.V[i]] = j;
            }
            else
            {
                g.V[j]--;
                g.E[g.V[j]] = i;
            }
        }
    }

    for (int i = 0; i < g.N; i++)
        qsort(g.E + g.V[i], g.V[i + 1] - g.V[i], sizeof(int), compare);

    for (int u = 0; u < g.N; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            g.W[i] = p.cm[v][u] - p.cm[u][v];
        }
    }

    for (int i = 0; i < g.N; i++)
        g.cc[i] = 0;

    g.cc_size[0] = g.N;

    *g.n_cc = 1;

    return g;
}

void dfas_free(dfas p)
{
    free(p.V);
    free(p.E);
    free(p.W);
    free(p.a);
    free(p.cc);
    free(p.cc_size);
    free(p.n_cc);
}

void explore(int *S, int *N, int *I, int *on_stack, int *index, int *lowlink, int *cc, int *cc_size, int *Nc,
             int u, int *V, int *E, int *a)
{
    index[u] = *I;
    lowlink[u] = *I;
    *I += 1;

    S[*N] = u;
    *N += 1;
    on_stack[u] = 1;

    for (int i = V[u]; i < V[u + 1]; i++)
    {
        if (!a[i])
            continue;

        int v = E[i];
        if (index[v] < 0)
        {
            explore(S, N, I, on_stack, index, lowlink, cc, cc_size, Nc, v, V, E, a);
            if (lowlink[v] < lowlink[u])
                lowlink[u] = lowlink[v];
        }
        else if (on_stack[v] && index[v] < lowlink[u])
        {
            lowlink[u] = index[v];
        }
    }

    if (lowlink[u] == index[u])
    {
        int v;
        cc_size[*Nc] = 0;
        do
        {
            *N -= 1;
            v = S[(*N)];
            on_stack[v] = 0;
            cc[v] = *Nc;
            cc_size[*Nc]++;
        } while (v != u);
        *Nc += 1;
    }
}

void dfas_reduction_cc(dfas p)
{
    int *S = malloc(sizeof(int) * p.N);
    int N = 0, I = 0;

    *p.n_cc = 0;

    int *on_stack = malloc(sizeof(int) * p.N);
    int *index = malloc(sizeof(int) * p.N);
    int *lowlink = malloc(sizeof(int) * p.N);

    for (int i = 0; i < p.N; i++)
    {
        on_stack[i] = 0;
        index[i] = -1;
    }

    for (int u = 0; u < p.N; u++)
    {
        if (index[u] < 0)
            explore(S, &N, &I, on_stack, index, lowlink, p.cc, p.cc_size, p.n_cc, u, p.V, p.E, p.a);
    }

    for (int u = 0; u < p.N; u++)
    {
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            int v = p.E[i];
            if (p.cc[u] != p.cc[v])
                p.a[i] = 0;
        }
    }

    free(S);
    free(on_stack);
    free(index);
    free(lowlink);
}

void dfas_reduction_degree_one(dfas p)
{
    int *degree_in = malloc(sizeof(int) * p.N);
    int *degree_out = malloc(sizeof(int) * p.N);

    for (int u = 0; u < p.N; u++)
        degree_in[u] = 0;

    for (int u = 0; u < p.N; u++)
    {
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            if (!p.a[i])
                continue;

            int v = p.E[i];
            degree_in[v]++;
            degree_out[u]++;
        }
    }

    for (int u = 0; u < p.N; u++)
    {
        if (degree_in[u] == 1)
        {
            int w = -1;
            for (int i = 0; i < p.V[p.N] && w < 0; i++)
                if (p.a[i] && p.E[i] == u)
                    w = p.W[i];

            for (int i = p.V[u]; i < p.V[u + 1]; i++)
            {
                if (!p.a[i])
                    continue;

                int v = p.E[i];
                if (p.W[i] <= w)
                {
                    p.a[i] = 0;
                }
            }
        }
        else
        {
            for (int i = p.V[u]; i < p.V[u + 1]; i++)
            {
                if (!p.a[i])
                    continue;

                int v = p.E[i];
                if (degree_out[v] == 1 && p.W[i] <= p.W[p.V[v]])
                {
                    p.a[i] = 0;
                }
            }
        }
    }

    free(degree_in);
    free(degree_out);
}

void dfas_store_dfvs(FILE *f, dfas p)
{
    int N = p.V[p.N];
    int M = 0;

    for (int u = 0; u < p.N; u++)
    {
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            if (!p.a[i])
                continue;
            int v = p.E[i];
            for (int j = p.V[v]; j < p.V[v + 1]; j++)
                if (p.a[i])
                    M++;
        }
    }

    fprintf(f, "%d %d 0\n", N, M);
    for (int u = 0; u < p.N; u++)
    {
        for (int i = p.V[u]; i < p.V[u + 1]; i++)
        {
            if (p.a[i])
            {
                int v = p.E[i];
                for (int j = p.V[v]; j < p.V[v + 1]; j++)
                    if (p.a[i])
                        fprintf(f, "%d ", j + 1);
            }
            fprintf(f, "\n");
        }
    }
}

typedef struct
{
    int N, M, av, ae;
    int *V, *E;
} cycles;

void push_cycle(cycles *c, int N, int *cycle)
{
    if (c->M + N >= c->ae)
    {
        c->ae *= 2;
        c->E = realloc(c->E, sizeof(int) * c->ae);
    }
    if (c->N + 1 >= c->av)
    {
        c->av *= 2;
        c->V = realloc(c->V, sizeof(int) * c->av);
    }
    for (int i = 0; i < N; i++)
        c->E[c->M + i] = cycle[i];

    c->M += N;
    c->N += 1;
    c->V[c->N] = c->M;
}

int dfas_explore(dfas g, cycles *c, int *sat, int d, int *e, int *v, int *marks, int *soft_marks, int max_l)
{
    int u = v[d];
    int res = 0;
    for (int i = g.V[u]; i < g.V[u + 1]; i++)
    {
        if (!g.a[i] || sat[i])
            continue;

        e[d] = i;
        int w = g.E[i];
        if (soft_marks[w]) // cycle
        {
            int offset = 0;
            while (v[offset] != w)
                offset++;
            int size = (d - offset) + 1;
            if (size <= max_l)
                push_cycle(c, size, e + offset);

            res++;
        }
        else if (!marks[w])
        {
            marks[w] = 1;
            soft_marks[w] = 1;
            v[d + 1] = w;
            res += dfas_explore(g, c, sat, d + 1, e, v, marks, soft_marks, max_l);
            soft_marks[w] = 0;
        }
    }

    return res;
}

void shuffle(int *array, size_t n)
{
    if (n > 1)
    {
        size_t i;
        for (i = 0; i < n - 1; i++)
        {
            size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
            int t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

int add_cycles(dfas g, cycles *c, int *sat, int max_l)
{
    int *marks = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        marks[i] = 0;

    int *soft_marks = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        soft_marks[i] = 0;

    int *order = malloc(sizeof(int) * g.N);
    for (int i = 0; i < g.N; i++)
        order[i] = i;
    shuffle(order, g.N);

    int *e = malloc(sizeof(int) * g.N);
    int *v = malloc(sizeof(int) * g.N);

    int n_cycles = 0;
    for (int i = 0; i < g.N; i++)
    {
        int u = order[i];
        if (marks[u])
            continue;

        v[0] = u;
        marks[u] = 1;
        soft_marks[u] = 1;
        n_cycles += dfas_explore(g, c, sat, 0, e, v, marks, soft_marks, max_l);
        soft_marks[u] = 0;
    }
    free(marks);
    free(soft_marks);
    free(e);
    free(v);
    return n_cycles;
}

void add_cycles_init(dfas g, cycles *c)
{
    int cycle[4];
    for (int u = 0; u < g.N; u++)
    {
        for (int i = g.V[u]; i < g.V[u + 1]; i++)
        {
            int v = g.E[i];
            if (!g.a[i] || v < u)
                continue;

            for (int j = g.V[v]; j < g.V[v + 1]; j++)
            {
                int w = g.E[j];
                if (!g.a[j] || w < u)
                    continue;

                for (int k = g.V[w]; k < g.V[w + 1]; k++)
                {
                    int x = g.E[k];
                    if (!g.a[k] || x < u || x == v)
                        continue;

                    if (x == u) // && drand48() > 0.5)
                    {
                        cycle[0] = i;
                        cycle[1] = j;
                        cycle[2] = k;
                        push_cycle(c, 3, cycle);
                    }
                    else if (x != u)
                    {
                        for (int l = g.V[x]; l < g.V[x + 1]; l++)
                        {
                            if (g.a[l] && g.E[l] == u && drand48() > 0.99)
                            {
                                cycle[0] = i;
                                cycle[1] = j;
                                cycle[2] = k;
                                cycle[3] = l;
                                push_cycle(c, 4, cycle);
                            }
                        }
                    }
                }
            }
        }
    }
}

void dfas_solve_sat(cycles c, dfas g, int *sat)
{
    int pid = getpid();
    char name[256];
    sprintf(name, "%d.wcnf", pid);
    FILE *f = fopen(name, "w");
    for (int i = 0; i < c.N; i++)
    {
        fprintf(f, "h ");
        for (int j = c.V[i]; j < c.V[i + 1]; j++)
            fprintf(f, "%d ", c.E[j] + 1);
        fprintf(f, "0\n");
    }
    for (int i = 0; i < g.V[g.N]; i++)
    {
        if (!g.a[i])
            continue;
        fprintf(f, "%d -%d 0\n", g.W[i], i + 1);
    }
    fclose(f);
    char command[256];
    sprintf(command, "timeout 120s ./uwrmaxsat %d.wcnf > %d.sol", pid, pid);
    int rc = system(command);
    if (rc != 0)
    {
        printf("Abort\n");
        sprintf(command, "rm -f %d.wcnf %d.sol", pid, pid);
        rc = system(command);
        exit(1);
    }
    sprintf(name, "%d.sol", pid);
    f = fopen(name, "r");

    for (int i = 0; i < g.V[g.N]; i++)
        sat[i] = 0;

    char *line = NULL;
    size_t len = 0;
    rc = getline(&line, &len, f);
    while (line[0] != 'v')
        rc = getline(&line, &len, f);

    int v, offset;
    char *data = line + 1;
    for (int i = 0; i < g.V[g.N]; i++)
    {
        rc = sscanf(data, " %d%n", &v, &offset);
        if (v > 0)
        {
            sat[v - 1] = 1;
        }
        data += offset;
    }

    free(line);

    sprintf(command, "rm -f %d.wcnf %d.sol", pid, pid);
    rc = system(command);
}

void dfas_solve(dfas g)
{
    cycles c = {.N = 0, .M = 0, .av = 16, .ae = 16};
    c.V = malloc(sizeof(int) * c.av);
    c.V[0] = 0;
    c.E = malloc(sizeof(int) * c.ae);

    int *sat = malloc(sizeof(int) * g.V[g.N]);
    for (int i = 0; i < g.V[g.N]; i++)
        sat[i] = 0;

    int max_l = 3;
    add_cycles_init(g, &c);
    int K = c.N; // add_cycles(g, &c, sat, max_l);

    while (K > 0)
    {
        // printf("%d %d %d\n", K, c.N, max_l);
        dfas_solve_sat(c, g, sat);

        int prev = c.N;
        K = add_cycles(g, &c, sat, max_l);
        while (prev == c.N && K > 0)
        {
            max_l++;
            K = add_cycles(g, &c, sat, max_l);
        }
    }

    for (int i = 0; i < g.V[g.N]; i++)
    {
        if (sat[i])
            g.a[i] = 0;
    }
}

void dfas_solve_cc(dfas p)
{
    int *new_label = malloc(sizeof(int) * p.V[p.N]);
    int *old_label = malloc(sizeof(int) * p.V[p.N]);

    int *new_vertex_label = malloc(sizeof(int) * p.N);

    for (int cc = 0; cc < *p.n_cc; cc++)
    {
        if (p.cc_size[cc] < 2)
            continue;
        for (int i = 0; i < p.N; i++)
            new_vertex_label[i] = -1;
        dfas pcc = {.N = 0};
        int M = 0;
        for (int u = 0; u < p.N; u++)
        {
            if (p.cc[u] != cc)
                continue;
            for (int i = p.V[u]; i < p.V[u + 1]; i++)
            {
                int v = p.E[i];
                if (!p.a[i])
                    continue;

                if (new_vertex_label[u] < 0)
                {
                    new_vertex_label[u] = pcc.N;
                    pcc.N++;
                }
                M++;
            }
        }
        pcc.V = malloc(sizeof(int) * (pcc.N + 1));
        pcc.E = malloc(sizeof(int) * M);
        pcc.W = malloc(sizeof(int) * M);
        pcc.a = malloc(sizeof(int) * M);

        pcc.V[0] = 0;
        M = 0;
        for (int u = 0; u < p.N; u++)
        {
            if (p.cc[u] != cc || new_vertex_label[u] < 0)
                continue;
            for (int i = p.V[u]; i < p.V[u + 1]; i++)
            {
                int v = p.E[i];
                if (!p.a[i])
                    continue;

                new_label[i] = M;
                old_label[M] = i;
                pcc.W[M] = p.W[i];
                pcc.a[M] = p.a[i];
                pcc.E[M] = new_vertex_label[v];
                M++;
            }
            pcc.V[new_vertex_label[u] + 1] = M;
        }

        dfas_solve(pcc);
        for (int i = 0; i < pcc.V[pcc.N]; i++)
            if (!pcc.a[i])
                p.a[old_label[i]] = 0;

        free(pcc.V);
        free(pcc.E);
        free(pcc.W);
        free(pcc.a);
    }

    free(new_label);
    free(old_label);
    free(new_vertex_label);
}
