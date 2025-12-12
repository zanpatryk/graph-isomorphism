#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "graph.h"
#include "algorithms.h"
#include "console.h"

static void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <command> <graph_file> [n] [--batch|-b]\n\n", prog_name);
    fprintf(stderr, "Commands:\n");
    fprintf(stderr, "  iso_exact <file> <n>       Find n subgraph isomorphisms (exact)\n");
    fprintf(stderr, "  iso_approx <file> <n>      Find n subgraph isomorphisms (heuristic)\n");
    fprintf(stderr, "  ext_exact <file> <n>       Find minimal extension for n isomorphisms (exact)\n");
    fprintf(stderr, "  ext_approx <file> <n>      Find minimal extension for n isomorphisms (heuristic)\n");
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  --batch, -b                Disable interactive mode (stop at n results)\n");
    fprintf(stderr, "\nExample:\n");
    fprintf(stderr, "  %s iso_exact data/graph.txt 3\n", prog_name);
    fprintf(stderr, "  %s ext_approx data/graph.txt 2 --batch\n", prog_name);
}

int main(const int argc, char *argv[]) {
    console_init();

    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *command = argv[1];
    const char *file_path = argv[2];
    int n = 1;
    bool interactive = true;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--batch") == 0 || strcmp(argv[i], "-b") == 0) {
            interactive = false;
        } else if (argv[i][0] != '-') {
            n = atoi(argv[i]);
            if (n < 1) n = 1;
        }
    }

    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;

    if (load_graphs(file_path, &n_g, &adj_g, &n_h, &adj_h) != 0) {
        return 1;
    }

    printf("Loaded G: %d vertices, H: %d vertices\n", n_g, n_h);
    print_adj_matrix("G", n_g, adj_g);
    print_adj_matrix("H", n_h, adj_h);

    if (strcmp(command, "iso_exact") == 0) {
        printf("\n=== Finding %d isomorphism(s) [EXACT]%s ===\n", n, interactive ? "" : " [BATCH]");
        IsomorphismResult *result = find_isomorphisms_exact(n_g, adj_g, n_h, adj_h, n, interactive);
        printf("\n--- Summary ---\n");
        printf("Total isomorphisms found: %d\n", result->num_found);
        printf("G is subgraph of H: %s\n", result->is_subgraph ? "YES" : "NO");
        free_isomorphism_result(result);

    } else if (strcmp(command, "iso_approx") == 0) {
        printf("\n=== Finding %d isomorphism(s) [HEURISTIC]%s ===\n", n, interactive ? "" : " [BATCH]");
        IsomorphismResult *result = find_isomorphisms_greedy(n_g, adj_g, n_h, adj_h, n, interactive);
        printf("\n--- Summary ---\n");
        printf("Total isomorphisms found: %d\n", result->num_found);
        printf("G is subgraph of H: %s\n", result->is_subgraph ? "YES" : "NO");
        free_isomorphism_result(result);

    } else if (strcmp(command, "ext_exact") == 0) {
        printf("\n=== Finding minimal extension for %d isomorphism(s) [EXACT]%s ===\n", n, interactive ? "" : " [BATCH]");
        ExtensionResult *result = find_minimal_extension_exact(n_g, adj_g, n_h, adj_h, n, interactive);
        printf("\n--- Summary ---\n");
        printf("Total mappings found: %d\n", result->num_mappings);
        printf("Total edges added: %d\n", result->total_edges_added);
        printf("\nFinal H' adjacency matrix:\n");
        print_matrix_highlighted(n_h, result->extended_adj_h, adj_h);
        free_extension_result(result);

    } else if (strcmp(command, "ext_approx") == 0) {
        printf("\n=== Finding minimal extension for %d isomorphism(s) [HEURISTIC]%s ===\n", n, interactive ? "" : " [BATCH]");
        ExtensionResult *result = find_minimal_extension_greedy(n_g, adj_g, n_h, adj_h, n, interactive);
        printf("\n--- Summary ---\n");
        printf("Total mappings found: %d\n", result->num_mappings);
        printf("Total edges added: %d\n", result->total_edges_added);
        printf("\nFinal H' adjacency matrix:\n");
        print_matrix_highlighted(n_h, result->extended_adj_h, adj_h);
        free_extension_result(result);

    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", command);
        print_usage(argv[0]);
        free(adj_g);
        free(adj_h);
        return 1;
    }

    free(adj_g);
    free(adj_h);
    return 0;
}