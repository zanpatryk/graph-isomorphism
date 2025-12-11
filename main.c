#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "algorithms.h"

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <command> <graph_file> [n]\n\n", prog_name);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  iso_exact <file> <n>       Find n subgraph isomorphisms (exact)\n");
    fprintf(stderr, "  iso_approx <file> <n>      Find n subgraph isomorphisms (heuristic)\n");
    fprintf(stderr, "  ext_exact <file> <n>       Find minimal extension for n isomorphisms (exact)\n");
    fprintf(stderr, "  ext_approx <file> <n>      Find minimal extension for n isomorphisms (heuristic)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Example:\n");
    fprintf(stderr, "  %s iso_exact data/triangle-square.txt 3\n", prog_name);
    fprintf(stderr, "  %s ext_approx data/compy.txt 2\n", prog_name);
}

int main(const int argc, char *argv[]) {
    // Check minimum arguments
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];
    const char *file_path = argv[2];
    int n = 1;  // Default

    // Parse n parameter if provided
    if (argc >= 4) {
        n = atoi(argv[3]);
        if (n < 1) {
            fprintf(stderr, "Error: n must be at least 1.\n");
            return 1;
        }
    }

    // Load graphs
    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;

    if (load_graphs(file_path, &n_g, &adj_g, &n_h, &adj_h) != 0) {
        return 1;
    }

    printf("Loaded G: %d vertices, H: %d vertices\n", n_g, n_h);
    print_adj_matrix("G", n_g, adj_g);
    print_adj_matrix("H", n_h, adj_h);

    // Execute command
    if (strcmp(command, "iso_exact") == 0) {
        // Find n isomorphisms using exact algorithm
        printf("\n=== Finding %d isomorphism(s) [EXACT] ===\n", n);
        IsomorphismResult *result = find_isomorphisms_exact(n_g, adj_g, n_h, adj_h, n);
        print_isomorphism_result(result);
        free_isomorphism_result(result);

    } else if (strcmp(command, "iso_approx") == 0) {
        // Find n isomorphisms using heuristic
        printf("\n=== Finding %d isomorphism(s) [HEURISTIC] ===\n", n);
        IsomorphismResult *result = find_isomorphisms_greedy(n_g, adj_g, n_h, adj_h, n);
        print_isomorphism_result(result);
        free_isomorphism_result(result);

    } else if (strcmp(command, "ext_exact") == 0) {
        // Find minimal extension using exact algorithm
        printf("\n=== Finding minimal extension for %d isomorphism(s) [EXACT] ===\n", n);
        ExtensionResult *result = find_minimal_extension_exact(n_g, adj_g, n_h, adj_h, n);
        print_extension_result(result, adj_g);
        free_extension_result(result);

    } else if (strcmp(command, "ext_approx") == 0) {
        // Find minimal extension using heuristic
        printf("\n=== Finding minimal extension for %d isomorphism(s) [HEURISTIC] ===\n", n);
        ExtensionResult *result = find_minimal_extension_greedy(n_g, adj_g, n_h, adj_h, n);
        print_extension_result(result, adj_g);
        free_extension_result(result);

    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", command);
        print_usage(argv[0]);
        free(adj_g);
        free(adj_h);
        return 1;
    }

    // Cleanup
    free(adj_g);
    free(adj_h);
    return 0;
}