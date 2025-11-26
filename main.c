#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "algorithms.h"

int main(const int argc, char *argv[]) {
    // 1. Check Arguments
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <function_name> <path_to_graph_data>\n", argv[0]);
        fprintf(stderr, "Available functions:\n");
        fprintf(stderr, "  minimal_extension\n");
        return 1;
    }

    const char *function_name = argv[1];
    const char *file_path = argv[2];

    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;

    // 2. Load Graphs (Using our safe, existing loader from graph.c)
    if (load_graphs(file_path, &n_g, &adj_g, &n_h, &adj_h) != 0) {
        // load_graphs prints its own error messages
        return 1;
    }

    // 3. Run Algorithm
    if (strcmp(function_name, "minimal_extension") == 0) {
        solve_minimal_extension(n_g, adj_g, n_h, adj_h);
    } else {
        fprintf(stderr, "Error: Unknown function '%s'\n", function_name);
        free(adj_g);
        free(adj_h);
        return 1;
    }

    // 4. Cleanup
    free(adj_g);
    free(adj_h);
    return 0;
}
