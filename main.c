#include <stdlib.h>
#include "graph.h"

int main(void) {
    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;
    const char *path = "data/graphs.txt";

    if (load_graphs(path, &n_g, &adj_g, &n_h, &adj_h) != 0) return 1;

    print_adj_matrix("g", n_g, adj_g);
    print_adj_matrix("h", n_h, adj_h);

    free(adj_g);
    free(adj_h);
    return 0;
}
