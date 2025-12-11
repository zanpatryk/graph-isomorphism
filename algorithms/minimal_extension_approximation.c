#include "minimal_extension_approximation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_MAPPINGS 4096

// ============================================================================
// Helper Structures
// ============================================================================

typedef struct {
    int id;
    int total_degree;  // in_degree + out_degree
} VertexInfo;

// ============================================================================
// Helper Functions
// ============================================================================

static inline int get_adj(const int *adj, int n, int i, int j) {
    return adj[i * n + j];
}

static inline void set_adj(int *adj, int n, int i, int j, int val) {
    adj[i * n + j] = val;
}

static int min(int a, int b) {
    return (a < b) ? a : b;
}

// Comparator: descending by degree, then ascending by id
static int compare_vertices(const void *a, const void *b) {
    const VertexInfo *va = (const VertexInfo *)a;
    const VertexInfo *vb = (const VertexInfo *)b;
    if (vb->total_degree != va->total_degree) {
        return vb->total_degree - va->total_degree;
    }
    return va->id - vb->id;
}

// Calculate degrees for all vertices
static void calc_degrees(int n, const int *adj, VertexInfo *infos) {
    for (int i = 0; i < n; i++) {
        infos[i].id = i;
        infos[i].total_degree = 0;

        // Out-degree
        for (int j = 0; j < n; j++) {
            infos[i].total_degree += get_adj(adj, n, i, j);
        }
        // In-degree
        for (int j = 0; j < n; j++) {
            infos[i].total_degree += get_adj(adj, n, j, i);
        }
    }
}

// Check if mapping already exists
static bool mapping_exists(int **existing, int num_existing, const int *mapping, int n_g) {
    for (int i = 0; i < num_existing; i++) {
        bool same = true;
        for (int v = 0; v < n_g && same; v++) {
            if (existing[i][v] != mapping[v]) same = false;
        }
        if (same) return true;
    }
    return false;
}

// ============================================================================
// Greedy Single Mapping Finder
// ============================================================================

// Run greedy from a fixed first assignment (first_v -> first_u)
static int *greedy_from_start(int n_g, const int *adj_g,
                               int n_h, const int *adj_h_current,
                               const VertexInfo *sorted_g,
                               const VertexInfo *h_info,
                               int first_v, int first_u,
                               int *out_deficit) {
    int *mapping = (int *)malloc(n_g * sizeof(int));
    for (int i = 0; i < n_g; i++) mapping[i] = -1;

    bool *used_h = (bool *)calloc(n_h, sizeof(bool));

    // Fix the first assignment
    mapping[first_v] = first_u;
    used_h[first_u] = true;

    // Greedy matching for remaining vertices
    for (int i = 0; i < n_g; i++) {
        int v = sorted_g[i].id;

        if (mapping[v] != -1) continue;  // Already assigned (first vertex)

        int best_u = -1;
        int best_score = -1;
        int best_h_degree = -1;

        for (int u = 0; u < n_h; u++) {
            if (used_h[u]) continue;

            int score = 0;

            for (int j = 0; j < n_g; j++) {
                if (mapping[j] != -1) {
                    int mapped_u = mapping[j];

                    int g_mult_in = get_adj(adj_g, n_g, j, v);
                    int h_mult_in = get_adj(adj_h_current, n_h, mapped_u, u);
                    score += min(g_mult_in, h_mult_in);

                    int g_mult_out = get_adj(adj_g, n_g, v, j);
                    int h_mult_out = get_adj(adj_h_current, n_h, u, mapped_u);
                    score += min(g_mult_out, h_mult_out);
                }
            }

            int g_loop = get_adj(adj_g, n_g, v, v);
            int h_loop = get_adj(adj_h_current, n_h, u, u);
            score += min(g_loop, h_loop);

            int u_deg = h_info[u].total_degree;
            if (score > best_score ||
                (score == best_score && u_deg > best_h_degree) ||
                (score == best_score && u_deg == best_h_degree && (best_u == -1 || u < best_u))) {
                best_score = score;
                best_u = u;
                best_h_degree = u_deg;
            }
        }

        if (best_u == -1) {
            free(mapping);
            free(used_h);
            *out_deficit = -1;
            return NULL;
        }

        mapping[v] = best_u;
        used_h[best_u] = true;
    }

    free(used_h);

    // Calculate deficit
    int deficit = 0;
    for (int i = 0; i < n_g; i++) {
        for (int j = 0; j < n_g; j++) {
            int g_mult = get_adj(adj_g, n_g, i, j);
            int h_mult = get_adj(adj_h_current, n_h, mapping[i], mapping[j]);
            if (h_mult < g_mult) {
                deficit += (g_mult - h_mult);
            }
        }
    }

    *out_deficit = deficit;
    return mapping;
}

// Find best greedy mapping by trying all possible first-vertex assignments
static int *find_greedy_mapping(int n_g, const int *adj_g,
                                 int n_h, const int *adj_h_current,
                                 int **existing_mappings, int num_existing,
                                 int *out_deficit) {
    // Sort G vertices by degree (descending)
    VertexInfo *sorted_g = (VertexInfo *)malloc(n_g * sizeof(VertexInfo));
    calc_degrees(n_g, adj_g, sorted_g);
    qsort(sorted_g, n_g, sizeof(VertexInfo), compare_vertices);

    // Pre-calculate H vertex degrees
    VertexInfo *h_info = (VertexInfo *)malloc(n_h * sizeof(VertexInfo));
    calc_degrees(n_h, adj_h_current, h_info);

    int *best_mapping = NULL;
    int best_deficit = -1;

    // Try each possible first-vertex assignment
    // Use the highest-degree vertex in G as the "anchor"
    int anchor_v = sorted_g[0].id;

    for (int u = 0; u < n_h; u++) {
        int deficit;
        int *mapping = greedy_from_start(n_g, adj_g, n_h, adj_h_current,
                                          sorted_g, h_info, anchor_v, u, &deficit);

        if (mapping == NULL) continue;

        // Check distinctness
        if (mapping_exists(existing_mappings, num_existing, mapping, n_g)) {
            free(mapping);
            continue;
        }

        // Keep if better
        if (best_mapping == NULL || deficit < best_deficit) {
            free(best_mapping);
            best_mapping = mapping;
            best_deficit = deficit;
        } else {
            free(mapping);
        }
    }

    free(sorted_g);
    free(h_info);

    *out_deficit = best_deficit;
    return best_mapping;
}

// Apply mapping edges to H'
static int apply_edges(int n_g, const int *adj_g, int n_h, int *adj_h, const int *mapping) {
    int added = 0;

    for (int i = 0; i < n_g; i++) {
        for (int j = 0; j < n_g; j++) {
            int g_mult = get_adj(adj_g, n_g, i, j);
            int u_i = mapping[i];
            int u_j = mapping[j];
            int h_mult = get_adj(adj_h, n_h, u_i, u_j);

            if (h_mult < g_mult) {
                int to_add = g_mult - h_mult;
                set_adj(adj_h, n_h, u_i, u_j, g_mult);
                added += to_add;
            }
        }
    }

    return added;
}

// ============================================================================
// Main Algorithm
// ============================================================================

ExtensionResult *find_minimal_extension_greedy(int n_g, const int *adj_g,
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

    if (n > MAX_MAPPINGS) {
        fprintf(stderr, "Warning: Limiting to %d mappings.\n", MAX_MAPPINGS);
        n = MAX_MAPPINGS;
    }

    // Iteratively find n mappings
    for (int k = 0; k < n; k++) {
        int deficit;
        int *new_mapping = find_greedy_mapping(n_g, adj_g, n_h, result->extended_adj_h,
                                                result->mappings, result->num_mappings,
                                                &deficit);

        if (new_mapping == NULL) {
            printf("Could only find %d distinct mappings (requested %d).\n", k, n);
            break;
        }

        // Store mapping
        result->mappings[result->num_mappings++] = new_mapping;

        // Apply edges
        int edges_this_round = apply_edges(n_g, adj_g, n_h, result->extended_adj_h, new_mapping);
        result->total_edges_added += edges_this_round;

        printf("Mapping %d: deficit = %d, edges added = %d\n", k + 1, deficit, edges_this_round);
    }

    return result;
}