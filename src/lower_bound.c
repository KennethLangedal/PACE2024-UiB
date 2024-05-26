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