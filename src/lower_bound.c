#include "lower_bound.h"

#include <stdlib.h>

typedef struct
{
    int n, w;
    int e[5];
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