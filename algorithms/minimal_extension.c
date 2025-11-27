#include "minimal_extension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_VERTICES 20
#define MAX_MAPPINGS 4096

// ============================================================================
// Internal State for Backtracking (single mapping)
// ============================================================================

static int g_min_deficit;
static int g_best_mapping[MAX_VERTICES];
static int g_n_g, g_n_h;
static const int *g_adj_g;
static int *g_adj_h_working;  // Working copy of H that we modify

// Already found mappings (to ensure distinctness)
static int g_found_mappings[MAX_MAPPINGS][MAX_VERTICES];
static int g_num_found;

// ============================================================================
// Helper Functions
// ============================================================================

static inline int get_adj(const int *adj, int n, int i, int j) {
    return adj[i * n + j];
}

static inline void set_adj(int *adj, int n, int i, int j, int val) {
    adj[i * n + j] = val;
}

// Check if mapping is distinct from all previously found
static bool is_distinct_mapping(const int *mapping) {
    for (int i = 0; i < g_num_found; i++) {
        bool same = true;
        for (int v = 0; v < g_n_g && same; v++) {
            if (g_found_mappings[i][v] != mapping[v]) {
                same = false;
            }
        }
        if (same) return false;
    }
    return true;
}

// Calculate deficit for a complete mapping against working H'
static int calculate_deficit(const int *mapping) {
    int deficit = 0;
    for (int i = 0; i < g_n_g; i++) {
        for (int j = 0; j < g_n_g; j++) {
            int g_mult = get_adj(g_adj_g, g_n_g, i, j);
            if (g_mult > 0) {
                int h_mult = get_adj(g_adj_h_working, g_n_h, mapping[i], mapping[j]);
                if (h_mult < g_mult) {
                    deficit += (g_mult - h_mult);
                }
            }
        }
    }
    return deficit;
}

// ============================================================================
// Backtracking to Find Single Best Mapping
// ============================================================================

static void backtrack_single(int v_idx, int *mapping, bool *used_h) {
    // Base case: all vertices mapped
    if (v_idx == g_n_g) {
        // Check distinctness
        if (!is_distinct_mapping(mapping)) {
            return;
        }

        int deficit = calculate_deficit(mapping);
        if (deficit < g_min_deficit) {
            g_min_deficit = deficit;
            memcpy(g_best_mapping, mapping, g_n_g * sizeof(int));
        }
        return;
    }

    // Try mapping v_idx to each unused vertex in H
    for (int u = 0; u < g_n_h; u++) {
        if (!used_h[u]) {
            mapping[v_idx] = u;
            used_h[u] = true;

            backtrack_single(v_idx + 1, mapping, used_h);

            used_h[u] = false;
        }
    }
}

// Find the best single mapping for current state of H'
static bool find_best_mapping(int *out_mapping, int *out_deficit) {
    g_min_deficit = INT_MAX;

    int *mapping = (int *)malloc(g_n_g * sizeof(int));
    bool *used_h = (bool *)calloc(g_n_h, sizeof(bool));

    backtrack_single(0, mapping, used_h);

    free(mapping);
    free(used_h);

    if (g_min_deficit == INT_MAX) {
        return false;  // No valid distinct mapping found
    }

    memcpy(out_mapping, g_best_mapping, g_n_g * sizeof(int));
    *out_deficit = g_min_deficit;
    return true;
}

// Apply mapping: add edges to working H' to satisfy this mapping
static int apply_mapping_edges(const int *mapping) {
    int edges_added = 0;

    for (int i = 0; i < g_n_g; i++) {
        for (int j = 0; j < g_n_g; j++) {
            int g_mult = get_adj(g_adj_g, g_n_g, i, j);
            int u_i = mapping[i];
            int u_j = mapping[j];
            int h_mult = get_adj(g_adj_h_working, g_n_h, u_i, u_j);

            if (h_mult < g_mult) {
                int to_add = g_mult - h_mult;
                set_adj(g_adj_h_working, g_n_h, u_i, u_j, g_mult);
                edges_added += to_add;
            }
        }
    }

    return edges_added;
}

// ============================================================================
// Main Algorithm: Iterative Extension for n Isomorphisms
// ============================================================================

ExtensionResult *find_minimal_extension_exact(int n_g, const int *adj_g,
                                               int n_h, const int *adj_h,
                                               int n) {
    ExtensionResult *result = (ExtensionResult *)malloc(sizeof(ExtensionResult));
    result->mappings = (int **)malloc(n * sizeof(int *));
    result->num_mappings = 0;
    result->n_g = n_g;
    result->n_h = n_h;
    result->total_edges_added = 0;
    result->extended_adj_h = (int *)malloc((size_t)n_h * n_h * sizeof(int));
    memcpy(result->extended_adj_h, adj_h, (size_t)n_h * n_h * sizeof(int));

    // Validation
    if (n_g > n_h) {
        fprintf(stderr, "Error: G has more vertices than H.\n");
        return result;
    }

    if (n_g > MAX_VERTICES || n_h > MAX_VERTICES) {
        fprintf(stderr, "Error: Graph size exceeds limit of %d vertices.\n", MAX_VERTICES);
        return result;
    }

    if (n > MAX_MAPPINGS) {
        fprintf(stderr, "Warning: Requested %d mappings, limiting to %d.\n", n, MAX_MAPPINGS);
        n = MAX_MAPPINGS;
    }

    // Initialize global state
    g_n_g = n_g;
    g_n_h = n_h;
    g_adj_g = adj_g;
    g_adj_h_working = result->extended_adj_h;  // We modify this in place
    g_num_found = 0;

    // Iteratively find n mappings
    for (int k = 0; k < n; k++) {
        int *new_mapping = (int *)malloc(n_g * sizeof(int));
        int deficit;

        if (!find_best_mapping(new_mapping, &deficit)) {
            // No more distinct mappings possible
            free(new_mapping);
            printf("Could only find %d distinct mappings (requested %d).\n", k, n);
            break;
        }

        // Store this mapping
        result->mappings[result->num_mappings] = new_mapping;
        memcpy(g_found_mappings[g_num_found], new_mapping, n_g * sizeof(int));
        g_num_found++;
        result->num_mappings++;

        // Apply edges to H' for this mapping
        int edges_this_round = apply_mapping_edges(new_mapping);
        result->total_edges_added += edges_this_round;

        printf("Mapping %d: deficit = %d, edges added this round = %d\n",
               k + 1, deficit, edges_this_round);
    }

    return result;
}

// ============================================================================
// Utility Functions
// ============================================================================

void free_extension_result(ExtensionResult *result) {
    if (result) {
        for (int i = 0; i < result->num_mappings; i++) {
            free(result->mappings[i]);
        }
        free(result->mappings);
        free(result->extended_adj_h);
        free(result);
    }
}

void print_extension_result(const ExtensionResult *result, const int *adj_g) {
    printf("\n--- Minimal Extension Result ---\n");
    printf("Total edges added: %d\n", result->total_edges_added);
    printf("Mappings found: %d\n", result->num_mappings);

    for (int k = 0; k < result->num_mappings; k++) {
        printf("\nMapping %d (G -> H):\n", k + 1);
        for (int v = 0; v < result->n_g; v++) {
            printf("    G_%d -> H_%d\n", v + 1, result->mappings[k][v] + 1);
        }
    }

    printf("\nEdges added to H:\n");
    if (result->total_edges_added == 0) {
        printf("    None. G is already embeddable in H.\n");
    } else {
        // Compare original H (we need to reconstruct it) with extended H'
        // Actually, we don't have original H here. We'll print based on what mappings needed.
        printf("    (See extended adjacency matrix)\n");
    }

    printf("\nExtended H' adjacency matrix:\n");
    printf("    ");
    for (int c = 0; c < result->n_h; c++) printf("%4d", c + 1);
    printf("\n");
    for (int r = 0; r < result->n_h; r++) {
        printf("%4d", r + 1);
        for (int c = 0; c < result->n_h; c++) {
            printf("%4d", get_adj(result->extended_adj_h, result->n_h, r, c));
        }
        printf("\n");
    }
}