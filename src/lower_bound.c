#include "lower_bound.h"
#include "glpk.h"

#include <stdlib.h>

typedef struct
{
    int n, w;
    int e[8];
} cycle;

void set_weight(int *W, cycle *c)
{
    int min_w = 999999;
    for (int i = 0; i < c->n; i++)
        min_w = min_w <= W[c->e[i]] ? min_w : W[c->e[i]];
    c->w = min_w;
}

int lower_bound_greedy(comp c)
{
    // for (int i = 0; i < c.n; i++)
    // {
    //     for (int j = 0; j < c.n; j++)
    //         printf("%d ", c.W[c.S[i]][c.S[j]]);
    //     printf("\n");
    // }
    // return 0;

    cycle *C = malloc(sizeof(cycle) * 1000000);
    int V = 0;
    for (int i = 0; i < c.n; i++)
    {
        int u = c.S[i];
        for (int j = i + 1; j < c.n; j++)
        {
            int v = c.S[j];
            if (c.W[v][u] == 0)
                continue;

            int n = 0;
            for (int k = i + 1; k < j; k++)
            {
                int w = c.S[k];
                if (c.W[u][w] == 0)
                    continue;

                if (c.W[w][v] > 0)
                {
                    C[n] = (cycle){.n = 3, .e = {u * c.n + w, w * c.n + v, v * c.n + u}};
                    set_weight(*c.W, C + n);
                    n++;
                    continue;
                }

                for (int l = k + 1; l < j; l++)
                {
                    int x = c.S[l];
                    if (c.W[w][x] == 0)
                        continue;

                    if (c.W[x][v] > 0)
                    {
                        C[n] = (cycle){.n = 4, .e = {u * c.n + w, w * c.n + x, x * c.n + v, v * c.n + u}};
                        set_weight(*c.W, C + n);
                        n++;
                        continue;
                    }

                    for (int m = l + 1; m < j; m++)
                    {
                        int y = c.S[m];
                        if (c.W[y][x] == 0)
                            continue;

                        if (c.W[y][v] > 0)
                        {
                            C[n] = (cycle){.n = 5, .e = {u * c.n + w, w * c.n + x, x * c.n + y, y * c.n + v, v * c.n + u}};
                            set_weight(*c.W, C + n);
                            n++;
                            continue;
                        }
                    }
                }
            }
            V += n;
            int min = 0;
            for (int k = 0; k < n; k++)
            {
                if (C[k].w == c.W[v][u])
                    min++;
            }

            printf("%d %d (%d %d) %d (%d)\n", u, v, i, j, n, min);
            // printf("Back edge: %d %d\n", c.W[v][u], V);
        }
    }

    printf("Could be enough with %d cycles\n", V);
    free(C);
}

/*
    int back_edges = 0, min_in_cycle = 0, nodes_in_lb_graph = 0;
    int max_gap = 0;
    for (int i = 0; i < c.n; i++)
    {
        int u = c.S[i];
        for (int j = i + 1; j < c.n; j++)
        {
            int v = c.S[j];
            if (c.W[v][u] == 0)
                continue;

            if ((j - i) + 1 > max_gap)
                max_gap = (j - i) + 1;

            back_edges++;
            int counter = 0;
            int in_c = 0;
            for (int k = i + 1; k < j; k++)
            {
                int w = c.S[k];
                if (c.W[u][w] == 0)
                    continue;

                if (c.W[w][v] != 0)
                {
                    if (c.W[v][u] <= c.W[u][w] && c.W[v][u] <= c.W[w][v])
                        in_c |= 1;
                    counter += c.W[u][w] < c.W[w][v] ? c.W[u][w] : c.W[w][v];
                }
                else
                {
                    for (int l = k + 1; l < j; l++)
                    {
                        int x = c.S[l];
                        if (c.W[w][x] == 0 || c.W[x][v] == 0 || c.W[u][x] != 0)
                            continue;

                        if (c.W[v][u] <= c.W[u][w] && c.W[v][u] <= c.W[w][x] && c.W[v][u] <= c.W[x][v])
                            in_c |= 1;

                        if (c.W[u][w] <= c.W[w][x] && c.W[u][w] <= c.W[x][v])
                            counter += c.W[u][w];
                        else if (c.W[w][x] <= c.W[u][w] && c.W[w][x] <= c.W[x][v])
                            counter += c.W[u][w];
                        else
                            counter += c.W[x][v];
                    }
                }

                // else if (c.W[w][v] == 0)
                // {
                //     for (int l = k + 1; l < j; l++)
                //     {
                //         int x = c.S[l];
                //         if (c.W[w][x] < c.W[v][u] || c.W[x][v] < c.W[v][u])
                //             continue;

                //         in_c |= 1;
                //         nodes_in_lb_graph++;
                //     }
                // }
            }
            if (in_c)
                min_in_cycle++;
            if (counter >= c.W[v][u])
                nodes_in_lb_graph++;
        }
    }
    printf("%d back edges (%d optimal edge in some cycle) and %d nodes in LB graph\n", back_edges, min_in_cycle, nodes_in_lb_graph);
    printf("Largest gap %d\n", max_gap);

    return 0;
*/

int lower_bound_flow_check(comp c)
{
}

void lb_explore(int **C, int n, int *order, int *visided, int *on_stack, int *prev, int *cycle, int u, int *lb, int max_length)
{
    visided[u] = 1;
    on_stack[u] = 1;
    for (int _i = 0; _i < n; _i++)
    {
        int v = order[_i];
        if (C[u][v] == 0)
            continue;

        if (on_stack[v])
        {
            int min = C[u][v];
            int m = 0, x = u, y = v;
            do
            {
                cycle[m++] = x * n + y;
                min = min > C[x][y] ? C[x][y] : min;
                y = x;
                x = prev[x];
            } while (y != v);

            if (min > 0 && m <= max_length)
            {
                *lb += min;
                for (int i = 0; i < m; i++)
                    (*C)[cycle[i]] -= min;
            }
        }
        else if (!visided[v])
        {
            prev[v] = u;
            lb_explore(C, n, order, visided, on_stack, prev, cycle, v, lb, max_length);
        }
    }
    on_stack[u] = 0;
}

void lb_shuffle(int *array, int n)
{
    for (int i = 0; i < n - 1; i++)
    {
        int j = i + rand() / (RAND_MAX / (n - i) + 1);
        int t = array[j];
        array[j] = array[i];
        array[i] = t;
    }
}

int lower_bound_cycle_packing(comp c)
{
    int *data = malloc(sizeof(int) * c.n * c.n);
    int **C = malloc(sizeof(int *) * c.n);
    for (int i = 0; i < c.n; i++)
        C[i] = data + i * c.n;

    for (int i = 0; i < c.n; i++)
        for (int j = 0; j < c.n; j++)
            C[i][j] = c.W[i][j];

    int *visited = malloc(sizeof(int) * c.n);
    int *on_stack = malloc(sizeof(int) * c.n);
    int *prev = malloc(sizeof(int) * c.n);
    int *cycle = malloc(sizeof(int) * c.n);
    int *order = malloc(sizeof(int) * c.n);
    for (int i = 0; i < c.n; i++)
        order[i] = i;

    lb_shuffle(order, c.n);

    int lb = 0;

    int max_length = 3;
    while (max_length < c.n)
    {
        for (int i = 0; i < c.n; i++)
        {
            visited[i] = 0;
            on_stack[i] = 0;
            prev[i] = -1;
        }

        int before = lb;
        for (int u = 0; u < c.n; u++)
            if (!visited[order[u]])
                lb_explore(C, c.n, order, visited, on_stack, prev, cycle, order[u], &lb, max_length);

        if (before == lb)
            max_length++;
    }

    free(data);
    free(C);
    free(visited);
    free(on_stack);
    free(prev);
    free(cycle);
    free(order);

    return lb;
}

const int MC = 4000;

void lp_explore(int **C, int **E, int n, int *visided, int *on_stack, int *prev, cycle *S, int *s, int u, int max_length)
{
    visided[u] = 1;
    on_stack[u] = 1;
    for (int v = 0; v < n; v++)
    {
        if (C[u][v] == 0 || E[u][v] >= MC * C[u][v])
            continue;

        if (on_stack[v])
        {
            int m = 0, x = u, y = v;
            cycle c = {.n = 0};
            do
            {
                if (E[x][y] >= MC * C[x][y])
                    break;
                c.e[m++] = x * n + y;
                y = x;
                x = prev[x];
            } while (y != v && m < max_length);

            if (y == v)
            {
                S[(*s)++] = c;
                for (int i = 0; i < m; i++)
                    (*E)[c.e[i]]++;
            }
        }
        else if (!visided[v])
        {
            prev[v] = u;
            lp_explore(C, E, n, visided, on_stack, prev, S, s, v, max_length);
        }
    }
    on_stack[u] = 0;
}

void lower_bound_cycle_packing_lp(comp c)
{
    const int max_c = 50000000;
    int n = 0;
    int ne = 0;
    cycle *C = malloc(sizeof(cycle) * max_c);

    int *_E = malloc(sizeof(int) * c.n * c.n);
    int **E = malloc(sizeof(int *) * c.n);
    for (int i = 0; i < c.n; i++)
        E[i] = _E + i * c.n;

    for (int i = 0; i < c.n * c.n; i++)
        _E[i] = 0;

    for (int u = 0; u < c.n; u++)
    {
        for (int v = u + 1; v < c.n; v++)
        {
            if (c.W[u][v] == 0)
                continue;

            for (int w = u + 1; w < c.n; w++)
            {
                if (c.W[v][w] == 0)
                    continue;

                if (c.W[w][u] > 0)
                {
                    if (E[u][v] < MC * c.W[u][v] && E[v][w] < MC * c.W[v][w] && MC * E[w][u] < MC * c.W[w][u])
                    {
                        E[u][v]++;
                        E[v][w]++;
                        E[w][u]++;
                        int e1 = u * c.n + v, e2 = v * c.n + w, e3 = w * c.n + u;
                        C[n++] = (cycle){.n = 3, .e = {e1, e2, e3}};
                        // set_weight(*c.W, &C[n - 1]);
                        ne += 3;
                    }
                }

                for (int x = u + 1; x < c.n; x++)
                {
                    if (c.W[w][x] == 0)
                        continue;

                    if (c.W[x][u] > 0)
                    {
                        if (E[u][v] < MC * c.W[u][v] && E[v][w] < MC * c.W[v][w] && E[w][x] < MC * c.W[w][x] && E[x][u] < MC * c.W[x][u])
                        {
                            E[u][v]++;
                            E[v][w]++;
                            E[w][x]++;
                            E[x][u]++;
                            C[n++] = (cycle){.n = 4, .e = {u * c.n + v, v * c.n + w, w * c.n + x, x * c.n + u}};
                            // set_weight(*c.W, &C[n - 1]);
                            ne += 4;
                        }
                    }

                    for (int y = u + 1; y < c.n; y++)
                    {
                        if (c.W[x][y] == 0)
                            continue;

                        if (c.W[y][u] > 0)
                        {
                            if (E[u][v] < MC * c.W[u][v] && E[v][w] < MC * c.W[v][w] && E[w][x] < MC * c.W[w][x] && E[x][y] < MC * c.W[x][y] && E[y][u] < MC * c.W[y][u])
                            {
                                E[u][v]++;
                                E[v][w]++;
                                E[w][x]++;
                                E[x][y]++;
                                E[y][u]++;
                                C[n++] = (cycle){.n = 5, .e = {u * c.n + v, v * c.n + w, w * c.n + x, x * c.n + y, y * c.n + u}};
                                // set_weight(*c.W, &C[n - 1]);
                                ne += 5;
                            }
                        }
                    }
                }
            }
        }
    }

    printf("%d\n", n);

    // int *visited = malloc(sizeof(int) * c.n);
    // int *on_stack = malloc(sizeof(int) * c.n);
    // int *prev = malloc(sizeof(int) * c.n);
    // for (int i = 0; i < c.n; i++)
    // {
    //     visited[i] = 0;
    //     on_stack[i] = 0;
    //     prev[i] = -1;
    // }

    // for (int u = 0; u < c.n; u++)
    //     if (!visited[u])
    //         lp_explore(c.W, E, c.n, visited, on_stack, prev, C, &n, u, 8);

    // printf("%d\n", n);

    glp_prob *lp = glp_create_prob();
    // glp_smcp parm;
    // parm.msg_lev = GLP_MSG_ON;
    // parm.meth = GLP_PRIMAL;
    // parm.pricing = GLP_PT_STD;
    // parm.r_test = GLP_RT_HAR;
    // parm.tol_bnd = 1e-7;
    // parm.tol_dj = 1e-7;
    // parm.tol_piv = 1e-10;
    // parm.obj_ll = -__DBL_MAX__;
    // parm.obj_ul = +__DBL_MAX__;
    // parm.it_lim = __INT_MAX__;
    // parm.tm_lim = __INT_MAX__;
    // parm.out_frq = 200;
    // parm.out_dly = 0;
    // parm.presolve = GLP_ON;
    int *ia = malloc(sizeof(int) * (ne + 1));
    int *ja = malloc(sizeof(int) * (ne + 1));
    double *ar = malloc(sizeof(double) * (ne + 1));

    glp_set_obj_dir(lp, GLP_MAX);
    glp_add_rows(lp, c.n * c.n);
    for (int i = 0; i < c.n * c.n; i++)
        glp_set_row_bnds(lp, i + 1, GLP_UP, 0.0, (*c.W)[i]);

    glp_add_cols(lp, n);
    int m = 0;

    for (int i = 0; i < n; i++)
    {
        glp_set_col_bnds(lp, i + 1, GLP_LO, 0.0, 0.0);
        glp_set_obj_coef(lp, i + 1, 1.0);

        for (int j = 0; j < C[i].n; j++)
        {
            ia[m + 1] = C[i].e[j] + 1;
            ja[m + 1] = i + 1;
            ar[m + 1] = 1.0;
            m++;
        }
    }

    glp_load_matrix(lp, ne, ia, ja, ar);
    glp_simplex(lp, NULL);
    double z = glp_get_obj_val(lp);

    printf("%lf\n", z);

    glp_delete_prob(lp);
    free(ia);
    free(ja);
    free(ar);
    free(E);
    free(_E);

    free(C);
}