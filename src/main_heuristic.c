#include "graph.h"
#include "heuristics.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    int *s = malloc(sizeof(int) * g.B);
    int k = simulated_annealing(g, s);
    printf("%d\n", k);
    f = fopen(argv[2], "w");
    for (int i = 0; i < g.B; i++)
        fprintf(f, "%d\n", s[i] + 1);
    fclose(f);

    free_graph(g);
    free(s);

    return 0;
}