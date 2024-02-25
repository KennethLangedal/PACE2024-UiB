#include "bnb.h"
#include "graph.h"
#include "ocm.h"

#include <stdio.h>
#include <stdlib.h>

#include <dirent.h>

int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    graph g = parse_graph(f);
    fclose(f);

    int *s = solve_bnb(g);

    // printf("%d\n", count_crossings(g, s));

    f = fopen(argv[2], "w");
    for (int i = 0; i < g.B; i++)
        fprintf(f, "%d\n", s[i] + 1);
    fclose(f);

    free_graph(g);
    free(s);

    return 0;
}