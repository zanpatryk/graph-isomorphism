#include "minimal_extension_approximation.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Struct to help sort vertices by degree
typedef struct {
    int id;
    int weighted_degree; // Sum of all edge weights (multiplicity)
    int distinct_neighbors; // Count of unique vertices connected to
} VertexInfo;

// Comparator for qsort (Descending order)
static int compare_vertices(const void *a, const void *b) {
    const VertexInfo *v1 = (const VertexInfo *)a;
    const VertexInfo *v2 = (const VertexInfo *)b;

    // 1. Primary Criteria: Weighted Degree (Total "traffic")
    if (v2->weighted_degree != v1->weighted_degree) {
        return v2->weighted_degree - v1->weighted_degree;
    }

    // 2. Secondary Criteria: Distinct Neighbors (Topological complexity)
    // If weights are equal, the one connecting to MORE unique people comes first.
    return v2->distinct_neighbors - v1->distinct_neighbors;
}

// Calculate degrees for all vertices in G
static void calculate_degrees(const int n, const int *flat_adj, VertexInfo *infos) {
    if (infos == NULL) {
        fprintf(stderr, "Error: infos pointer was null in minimal_extension_approximation/calculate_degrees. Exiting...\n");
        exit(1);
    }
    for (int i = 0; i < n; i++) {
        infos[i].id = i;
        infos[i].weighted_degree = 0;
        infos[i].distinct_neighbors = 0;
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            const int weight = flat_adj[i * n + j];
            if (weight > 0) {
                // Update Weighted Degree (Total Multiplicity)
                infos[i].weighted_degree += weight; // Out-degree
                infos[j].weighted_degree += weight; // In-degree

                // Update Distinct Neighbor Count
                // (Constraint: How many different nodes do I touch?)
                infos[i].distinct_neighbors++;
                infos[j].distinct_neighbors++;
            }
        }
    }
}

static int min(int a, int b) {
    return (a < b) ? a : b;
}

void solve_approximate_extension(const int n_g, const int *flat_adj_g, const int n_h, const int *flat_adj_h) {
    printf("Running Greedy Approximation Algorithm (Weighted + Distinct Heuristic)...\n");

    // 1. Validation
    if (n_g > n_h) {
        fprintf(stderr, "Error: G (%d) is larger than H (%d). Impossible.\n", n_g, n_h);
        return;
    }

    // 2. Setup Data Structures
    int *mapping = (int *) malloc(n_g * sizeof(int));
    for (int i = 0; i < n_g; i++) mapping[i] = -1; // -1 means not yet mapped

    bool *used_u = (bool *) calloc(n_h, sizeof(bool));

    // 3. Sort G vertices by Degree
    VertexInfo *sorted_g = (VertexInfo *) malloc(n_g * sizeof(VertexInfo));
    calculate_degrees(n_g, flat_adj_g, sorted_g);
    qsort(sorted_g, n_g, sizeof(VertexInfo), compare_vertices);

    // 4. The Greedy Loop
    for (int i = 0; i < n_g; i++) {
        const int v_curr = sorted_g[i].id;

        int best_u = -1;
        int best_score = -1;

        // Try every available node in H
        for (int u_cand = 0; u_cand < n_h; u_cand++) {
            if (used_u[u_cand]) continue;

            int current_score = 0;

            // Compare against all vertices of G that are ALREADY mapped
            for (int v_mapped = 0; v_mapped < n_g; v_mapped++) {
                if (mapping[v_mapped] != -1) {
                    const int u_mapped = mapping[v_mapped];

                    // Check edges: v_mapped -> v_curr vs u_mapped -> u_cand
                    const int g_mult_in = flat_adj_g[v_mapped * n_g + v_curr];
                    const int h_mult_in = flat_adj_h[u_mapped * n_h + u_cand];
                    current_score += min(g_mult_in, h_mult_in);

                    // Check edges: v_curr -> v_mapped vs u_cand -> u_mapped
                    const int g_mult_out = flat_adj_g[v_curr * n_g + v_mapped];
                    const int h_mult_out = flat_adj_h[u_cand * n_h + u_mapped];
                    current_score += min(g_mult_out, h_mult_out);
                }
            }

            // Check Self Loops
            const int g_loop = flat_adj_g[v_curr * n_g + v_curr];
            const int h_loop = flat_adj_h[u_cand * n_h + u_cand];
            current_score += min(g_loop, h_loop);

            // Maximization Step
            if (current_score > best_score) {
                best_score = current_score;
                best_u = u_cand;
            }
        }

        // Lock in the best choice
        if (best_u != -1) {
            mapping[v_curr] = best_u;
            used_u[best_u] = true;
        } else {
            fprintf(stderr, "Warning: Could not find a mapping for G node %d\n", v_curr);
        }
    }

    // 5. Calculate Final Deficit
    int total_edges_to_add = 0;

    printf("\n--- Approximation Result ---\n");

    // Print Mapping
    printf("1. Vertex Mapping (G -> H):\n");
    for (int i = 0; i < n_g; i++) {
        printf("    G_%d -> H_%d\n", i + 1, mapping[i] + 1);
    }

    // Print Edges to Add
    printf("\n2. Edges to Add to H:\n");
    bool perfect = true;
    for (int i = 0; i < n_g; i++) {
        for (int j = 0; j < n_g; j++) {
            if (flat_adj_g[i * n_g + j] > 0) {
                const int u = mapping[i];
                const int v = mapping[j];

                const int required = flat_adj_g[i * n_g + j];
                const int existing = flat_adj_h[u * n_h + v];

                if (existing < required) {
                    perfect = false;
                    const int missing = required - existing;
                    total_edges_to_add += missing;
                    printf("    Add %d edge(s): H_%d -> H_%d  (for G_%d -> G_%d)\n",
                           missing, u + 1, v + 1, i + 1, j + 1);
                }
            }
        }
    }

    if (perfect) {
        printf("    None. Perfect subgraph found.\n");
    }
    printf("\nTotal edges to add (Approximate): %d\n", total_edges_to_add);

    // Cleanup
    free(mapping);
    free(used_u);
    free(sorted_g);
}