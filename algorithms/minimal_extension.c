#include "minimal_extension.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_VERTICES 20

static int min_edges_to_add = INT_MAX;
static int best_mapping[MAX_VERTICES];
static int G_adj[MAX_VERTICES][MAX_VERTICES];
static int H_adj[MAX_VERTICES][MAX_VERTICES];
static int n_g_global;
static int n_h_global;

/**
 * Calculates cost for the current mapping.
 */
static int calculate_extension_cost(const int *current_mapping) {
    int current_edges_to_add = 0;
    for (int i = 0; i < n_g_global; i++) {
        for (int j = 0; j < n_g_global; j++) {
            if (G_adj[i][j] > 0) {
                const int u_mapped = current_mapping[i];
                const int v_mapped = current_mapping[j];
                const int g_mult = G_adj[i][j];
                const int h_mult = H_adj[u_mapped][v_mapped];

                if (h_mult < g_mult) {
                    current_edges_to_add += g_mult - h_mult;
                }
            }
        }
    }
    return current_edges_to_add;
}

/**
 * Recursive Backtracking with STRICT 1-to-1 Constraint
 */
static int backtrack_extend(const int v_index, int *mapping, bool *used_u) {
    if (mapping == NULL || used_u == NULL)
        return 1;

    // --- Base Case: All vertices in G are mapped ---
    if (v_index == n_g_global) {
        // NOW it is safe to calculate cost because 'mapping' is fully filled
        const int cost = calculate_extension_cost(mapping);
        if (cost < min_edges_to_add) {
            min_edges_to_add = cost;
            for (int i = 0; i < n_g_global; i++) {
                best_mapping[i] = mapping[i];
            }
        }
        return 0;
    }

    // --- Recursive Step ---
    for (int u = 0; u < n_h_global; u++) {
        // Strict 1-to-1 check
        if (!used_u[u]) {
            mapping[v_index] = u;
            used_u[u] = true;

            backtrack_extend(v_index + 1, mapping, used_u);

            used_u[u] = false; // Backtrack
        }
    }
    return 0;
}

static void print_solution_details() {
    printf("\n--- Minimal Extension Result ---\n");
    printf("Minimal number of added edges: %d\n", min_edges_to_add);

    printf("\n1. Vertex Mapping (G -> H):\n");
    for (int i = 0; i < n_g_global; i++) {
        printf("    G_%d -> H_%d\n", i + 1, best_mapping[i] + 1);
    }

    printf("\n2. Edges to Add to H:\n");
    if (min_edges_to_add == 0) {
        printf("    None. G is already a subgraph of H.\n");
    } else {
        for (int i = 0; i < n_g_global; i++) {
            for (int j = 0; j < n_g_global; j++) {
                if (G_adj[i][j] > 0) {
                    const int u = best_mapping[i];
                    const int v = best_mapping[j];
                    const int required = G_adj[i][j];
                    const int existing = H_adj[u][v];

                    if (existing < required) {
                        const int missing = required - existing;
                        printf("    Add %d edge(s): H_%d -> H_%d  (for G_%d -> G_%d)\n",
                               missing, u + 1, v + 1, i + 1, j + 1);
                    }
                }
            }
        }
    }
}

void solve_minimal_extension(const int n_g, const int *flat_adj_g, const int n_h, const int *flat_adj_h) {
    printf("Running Minimal Extension Algorithm...\n");

    // 1. Validation
    if (n_g > MAX_VERTICES || n_h > MAX_VERTICES) {
        fprintf(stderr, "Error: Graph size exceeds hardcoded limit of %d vertices.\n", MAX_VERTICES);
        return;
    }
    if (n_g > n_h) {
        fprintf(stderr, "Error: G (%d vertices) is larger than H (%d vertices). Subgraph isomorphism impossible.\n",
                n_g, n_h);
        return;
    }

    // 2. Copy data from flat parser arrays to algorithm's 2D globals
    n_g_global = n_g;
    n_h_global = n_h;

    // Clear globals first
    memset(G_adj, 0, sizeof(G_adj));
    memset(H_adj, 0, sizeof(H_adj));

    // Copy G (flat_adj_g uses row*n + col)
    for (int r = 0; r < n_g; r++) {
        for (int c = 0; c < n_g; c++) {
            G_adj[r][c] = flat_adj_g[r * n_g + c];
        }
    }
    // Copy H
    for (int r = 0; r < n_h; r++) {
        for (int c = 0; c < n_h; c++) {
            H_adj[r][c] = flat_adj_h[r * n_h + c];
        }
    }

    // 3. Initialize Algorithm Variables
    int total_edges_G = 0;
    for (int i = 0; i < n_g; i++)
        for (int j = 0; j < n_g; j++)
            total_edges_G += G_adj[i][j];

    min_edges_to_add = total_edges_G; // Worst case init

    int *partial_mapping = (int *) malloc(n_g * sizeof(int));
    bool *used_u = (bool *) calloc(n_h, sizeof(bool));

    // 4. Run Backtracking
    backtrack_extend(0, partial_mapping, used_u);

    // 5. Print Results
    print_solution_details();

    // Cleanup local allocations
    free(partial_mapping);
    free(used_u);
}