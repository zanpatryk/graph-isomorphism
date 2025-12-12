#include "isomorphism.h"
#include "product_graph.h"
#include "../console.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ISOMORPHISMS 1000  // Safety limit

// ============================================================================
// Helper Functions
// ============================================================================

bool mappings_equal(const int *map1, const int *map2, int n_g) {
    for (int i = 0; i < n_g; i++) {
        if (map1[i] != map2[i]) return false;
    }
    return true;
}

// Extract mapping from clique (array of product graph vertex indices)
static int *extract_mapping(const ProductGraph *pg, const int *clique, int clique_size) {
    int *mapping = (int *) malloc(pg->n_g * sizeof(int));
    for (int i = 0; i < pg->n_g; i++) {
        mapping[i] = -1; // Initialize as unmapped
    }


    for (int i = 0; i < clique_size; i++) {
        int pg_idx = clique[i];
        int v = pg->vertices[pg_idx].v;
        int u = pg->vertices[pg_idx].u;
        mapping[v] = u;
    }

    return mapping;
}

// Check if mapping already exists in result
static bool mapping_exists(const IsomorphismResult *result, const int *mapping) {
    for (int i = 0; i < result->num_found; i++) {
        if (mappings_equal(result->mappings[i], mapping, result->n_g)) {
            return true;
        }
    }
    return false;
}

// Add mapping to result
static void add_mapping(IsomorphismResult *result, int *mapping) {
    if (result->num_found >= MAX_ISOMORPHISMS) {
        free(mapping);
        return;
    }
    result->mappings[result->num_found++] = mapping;
}

static void print_single_mapping(int idx, const int *mapping, int n_g,
                                 int n_h, const int *adj_g, const int *adj_h) {
    printf("\nMapping %d:\n", idx);
    for (int v = 0; v < n_g; v++) {
        printf("    G_%d -> H_%d\n", v + 1, mapping[v] + 1);
    }
    printf("\nH with mapped edges highlighted:\n");
    print_matrix_with_mapping(n_h, adj_h, NULL, n_g, adj_g, mapping);
}

// ============================================================================
// Exact Algorithm: Bron-Kerbosch with pivoting (simplified & fixed)
// ============================================================================

typedef struct {
    const ProductGraph *pg;
    IsomorphismResult *result;
    int target_size;
    int max_to_find;
    int max_pg_vertices; // For safe allocation
    int initial_target;
    bool interactive;
    bool stop_requested;
    const int *adj_g;
    const int *adj_h;
} BKContext;

// Check if vertex v is compatible with all vertices in clique R
static bool is_compatible_with_clique(const ProductGraph *pg, int v,
                                      const int *R, int r_size) {
    int v_g = pg->vertices[v].v;
    int v_h = pg->vertices[v].u;


    for (int i = 0; i < r_size; i++) {
        int r = R[i];
        int r_g = pg->vertices[r].v;
        int r_h = pg->vertices[r].u;

        // Check injectivity
        if (v_g == r_g || v_h == r_h) return false;

        // Check adjacency in product graph
        if (!product_graph_adjacent(pg, v, r)) return false;
    }
    return true;
}

// Simple recursive backtracking to find cliques of target size
static void find_cliques_recursive(BKContext *ctx,
                                   int *R, int r_size,
                                   int *candidates, int num_candidates) {
    if (ctx->stop_requested) return;

    // Check if we’ve found enough
    if (ctx->max_to_find > 0 && ctx->result->num_found >= ctx->max_to_find) {
        return;
    }


    // Found a clique of target size
    if (r_size == ctx->target_size) {
        int *mapping = extract_mapping(ctx->pg, R, r_size);
        if (!mapping_exists(ctx->result, mapping)) {
            add_mapping(ctx->result, mapping);
            ctx->result->is_subgraph = true;

            print_single_mapping(ctx->result->num_found, mapping, ctx->result->n_g, ctx->pg->n_h, ctx->adj_g,
                                 ctx->adj_h);

            // After reaching initial target, prompt for more
            if (ctx->result->num_found >= ctx->initial_target) {
                if (!ctx->interactive) {
                    ctx->stop_requested = true;
                } else if (!prompt_continue("Continue searching for more isomorphisms?")) {
                    ctx->stop_requested = true;
                }
            }
        } else {
            free(mapping);
        }
        return;
    }

    // Pruning: can't reach target size
    if (r_size + num_candidates < ctx->target_size) {
        return;
    }

    // Try each candidate
    for (int i = 0; i < num_candidates; i++) {
        // Check if we've found enough
        if (ctx->max_to_find > 0 && ctx->result->num_found >= ctx->max_to_find) {
            return;
        }

        int v = candidates[i];

        // Check if v is compatible with current clique
        if (!is_compatible_with_clique(ctx->pg, v, R, r_size)) {
            continue;
        }

        // Add v to clique
        R[r_size] = v;

        // Build new candidate set: vertices after i that are adjacent to v
        int *new_candidates = (int *) malloc(num_candidates * sizeof(int));
        int new_num = 0;

        for (int j = i + 1; j < num_candidates; j++) {
            int u = candidates[j];
            if (product_graph_adjacent(ctx->pg, v, u)) {
                // Also check that u doesn't conflict with extended clique
                int u_g = ctx->pg->vertices[u].v;
                int u_h = ctx->pg->vertices[u].u;
                int v_g = ctx->pg->vertices[v].v;
                int v_h = ctx->pg->vertices[v].u;

                if (u_g != v_g && u_h != v_h) {
                    new_candidates[new_num++] = u;
                }
            }
        }

        // Recurse
        find_cliques_recursive(ctx, R, r_size + 1, new_candidates, new_num);

        free(new_candidates);
    }
}

IsomorphismResult *find_isomorphisms_exact(int n_g, const int *adj_g,
                                           int n_h, const int *adj_h,
                                           int n, bool interactive) {
    IsomorphismResult *result = (IsomorphismResult *) malloc(sizeof(IsomorphismResult));
    result->mappings = (int **) malloc(MAX_ISOMORPHISMS * sizeof(int *));
    result->num_found = 0;
    result->n_g = n_g;
    result->is_subgraph = false;


    // Edge case
    if (n_g > n_h) {
        printf("G has more vertices than H. No isomorphism possible.\n");
        return result;
    }

    if (n_g == 0) {
        // Empty G is subgraph of any H
        result->is_subgraph = true;
        int *empty_mapping = (int *) calloc(1, sizeof(int));
        add_mapping(result, empty_mapping);
        return result;
    }

    // Build product graph
    ProductGraph *pg = build_product_graph(n_g, adj_g, n_h, adj_h);
    if (!pg) {
        printf("Failed to build product graph.\n");
        return result;
    }

    if (pg->num_vertices == 0) {
        printf("Product graph is empty. No isomorphism possible.\n");
        free_product_graph(pg);
        return result;
    }

    printf("Product graph has %d vertices\n", pg->num_vertices);

    // Initialize context
    BKContext ctx = {
        .pg = pg,
        .result = result,
        .target_size = n_g,
        .max_to_find = 0, // Changed: unlimited, we control via prompt
        .max_pg_vertices = pg->num_vertices,
        .initial_target = n,
        .interactive = interactive,
        .stop_requested = false,
        .adj_g = adj_g,
        .adj_h = adj_h
    };
    // Allocate working arrays
    int *R = (int *) malloc(pg->num_vertices * sizeof(int));
    int *candidates = (int *) malloc(pg->num_vertices * sizeof(int));

    // Initial candidates: all vertices in product graph
    for (int i = 0; i < pg->num_vertices; i++) {
        candidates[i] = i;
    }

    // Find cliques
    find_cliques_recursive(&ctx, R, 0, candidates, pg->num_vertices);

    free(R);
    free(candidates);
    free_product_graph(pg);

    return result;
}

// ============================================================================
// Heuristic Algorithm: Direct Vertex-by-Vertex Greedy Matching
// ============================================================================

// This approach is more reliable than greedy clique on product graph.
// It directly builds a mapping by greedily assigning vertices.

typedef struct {
    int id;
    int total_degree;
} GreedyVertexInfo;

static int compare_by_degree_desc(const void *a, const void *b) {
    const GreedyVertexInfo *va = (const GreedyVertexInfo *) a;
    const GreedyVertexInfo *vb = (const GreedyVertexInfo *) b;
    if (vb->total_degree != va->total_degree) {
        return vb->total_degree - va->total_degree;
    }
    return va->id - vb->id;
}

static inline int get_adj_val(const int *adj, int n, int i, int j) {
    return adj[i * n + j];
}

// Calculate total degree (in + out) for each vertex
static void calc_total_degrees(int n, const int *adj, GreedyVertexInfo *infos) {
    for (int i = 0; i < n; i++) {
        infos[i].id = i;
        infos[i].total_degree = 0;
        for (int j = 0; j < n; j++) {
            infos[i].total_degree += get_adj_val(adj, n, i, j); // out
            infos[i].total_degree += get_adj_val(adj, n, j, i); // in
        }
    }
}

// Check if mapping v -> u is valid given current partial mapping
// Returns true if all edges between v and already-mapped vertices are preserved
static bool is_valid_assignment(int v, int u,
                                int n_g, const int *adj_g,
                                int n_h, const int *adj_h,
                                const int *mapping) {
    // Check degree constraints
    int out_deg_g = 0, in_deg_g = 0;
    int out_deg_h = 0, in_deg_h = 0;


    for (int j = 0; j < n_g; j++) {
        out_deg_g += get_adj_val(adj_g, n_g, v, j);
        in_deg_g += get_adj_val(adj_g, n_g, j, v);
    }
    for (int j = 0; j < n_h; j++) {
        out_deg_h += get_adj_val(adj_h, n_h, u, j);
        in_deg_h += get_adj_val(adj_h, n_h, j, u);
    }

    if (out_deg_g > out_deg_h || in_deg_g > in_deg_h) {
        return false;
    }

    // Check edge preservation with already-mapped vertices
    for (int v2 = 0; v2 < n_g; v2++) {
        if (mapping[v2] == -1) continue; // Not yet mapped

        int u2 = mapping[v2];

        // Edge v -> v2 must be preserved as u -> u2
        int g_mult = get_adj_val(adj_g, n_g, v, v2);
        int h_mult = get_adj_val(adj_h, n_h, u, u2);
        if (g_mult > h_mult) return false;

        // Edge v2 -> v must be preserved as u2 -> u
        g_mult = get_adj_val(adj_g, n_g, v2, v);
        h_mult = get_adj_val(adj_h, n_h, u2, u);
        if (g_mult > h_mult) return false;
    }

    // Check self-loop
    int g_loop = get_adj_val(adj_g, n_g, v, v);
    int h_loop = get_adj_val(adj_h, n_h, u, u);
    if (g_loop > h_loop) return false;

    return true;
}

// Score an assignment (higher = more edges preserved = better)
static int score_assignment(int v, int u,
                            int n_g, const int *adj_g,
                            int n_h, const int *adj_h,
                            const int *mapping) {
    int score = 0;


    for (int v2 = 0; v2 < n_g; v2++) {
        if (mapping[v2] == -1) continue;

        int u2 = mapping[v2];

        // Edges preserved
        score += get_adj_val(adj_g, n_g, v, v2);
        score += get_adj_val(adj_g, n_g, v2, v);
    }

    // Self-loop
    score += get_adj_val(adj_g, n_g, v, v);

    return score;
}

// Try to build a valid isomorphism starting with first_v -> first_u
static int *try_greedy_from_start(int n_g, const int *adj_g,
                                  int n_h, const int *adj_h,
                                  const GreedyVertexInfo *sorted_g,
                                  int first_v, int first_u) {
    int *mapping = (int *) malloc(n_g * sizeof(int));
    for (int i = 0; i < n_g; i++) mapping[i] = -1;


    bool *used_h = (bool *) calloc(n_h, sizeof(bool));

    // Fix first assignment
    mapping[first_v] = first_u;
    used_h[first_u] = true;

    // Greedily assign remaining vertices
    for (int i = 0; i < n_g; i++) {
        int v = sorted_g[i].id;
        if (mapping[v] != -1) continue; // Already assigned

        int best_u = -1;
        int best_score = -1;

        for (int u = 0; u < n_h; u++) {
            if (used_h[u]) continue;

            if (!is_valid_assignment(v, u, n_g, adj_g, n_h, adj_h, mapping)) {
                continue;
            }

            int score = score_assignment(v, u, n_g, adj_g, n_h, adj_h, mapping);

            if (score > best_score) {
                best_score = score;
                best_u = u;
            }
        }

        if (best_u == -1) {
            // No valid assignment - this starting point failed
            free(mapping);
            free(used_h);
            return NULL;
        }

        mapping[v] = best_u;
        used_h[best_u] = true;
    }

    free(used_h);
    return mapping;
}

// Verify that a mapping is a valid subgraph isomorphism
static bool verify_isomorphism(int n_g, const int *adj_g,
                               int n_h, const int *adj_h,
                               const int *mapping) {
    for (int i = 0; i < n_g; i++) {
        for (int j = 0; j < n_g; j++) {
            int g_mult = get_adj_val(adj_g, n_g, i, j);
            int h_mult = get_adj_val(adj_h, n_h, mapping[i], mapping[j]);
            if (g_mult > h_mult) return false;
        }
    }
    return true;
}

IsomorphismResult *find_isomorphisms_greedy(int n_g, const int *adj_g,
                                            int n_h, const int *adj_h,
                                            int n, bool interactive) {
    IsomorphismResult *result = (IsomorphismResult *) malloc(sizeof(IsomorphismResult));
    result->mappings = (int **) malloc(MAX_ISOMORPHISMS * sizeof(int *));
    result->num_found = 0;
    result->n_g = n_g;
    result->is_subgraph = false;


    // Edge cases
    if (n_g > n_h) {
        printf("G has more vertices than H. No isomorphism possible.\n");
        return result;
    }

    if (n_g == 0) {
        result->is_subgraph = true;
        int *empty_mapping = (int *) calloc(1, sizeof(int));
        add_mapping(result, empty_mapping);
        return result;
    }

    // Sort G vertices by degree (descending) - high degree = more constrained
    GreedyVertexInfo *sorted_g = (GreedyVertexInfo *) malloc(n_g * sizeof(GreedyVertexInfo));
    calc_total_degrees(n_g, adj_g, sorted_g);
    qsort(sorted_g, n_g, sizeof(GreedyVertexInfo), compare_by_degree_desc);

    // Also sort H vertices by degree for smarter iteration
    GreedyVertexInfo *sorted_h = (GreedyVertexInfo *) malloc(n_h * sizeof(GreedyVertexInfo));
    calc_total_degrees(n_h, adj_h, sorted_h);
    qsort(sorted_h, n_h, sizeof(GreedyVertexInfo), compare_by_degree_desc);

    // The anchor vertex: highest degree in G
    int anchor_v = sorted_g[0].id;

    bool stop = false;

    /// Try each possible starting assignment for anchor vertex
    for (int h_idx = 0; h_idx < n_h && !stop; h_idx++) {
        int start_u = sorted_h[h_idx].id;

        int *mapping = try_greedy_from_start(n_g, adj_g, n_h, adj_h,
                                             sorted_g, anchor_v, start_u);

        if (mapping == NULL) continue;
        if (!verify_isomorphism(n_g, adj_g, n_h, adj_h, mapping)) {
            free(mapping);
            continue;
        }
        if (mapping_exists(result, mapping)) {
            free(mapping);
            continue;
        }

        add_mapping(result, mapping);
        result->is_subgraph = true;

        printf("Found isomorphism %d (anchor G_%d -> H_%d)\n",
               result->num_found, anchor_v + 1, start_u + 1);
        print_single_mapping(result->num_found, result->mappings[result->num_found - 1], n_g, n_h, adj_g, adj_h);

        if (result->num_found >= n) {
            if (!interactive) { stop = true; } else if (!prompt_continue("Continue searching for more isomorphisms?")) {
                stop = true;
            }
        }
    }

    // Try other anchor vertices
    for (int g_idx = 1; g_idx < n_g && !stop; g_idx++) {
        int alt_anchor = sorted_g[g_idx].id;

        for (int h_idx = 0; h_idx < n_h && !stop; h_idx++) {
            int start_u = sorted_h[h_idx].id;

            int *mapping = try_greedy_from_start(n_g, adj_g, n_h, adj_h,
                                                 sorted_g, alt_anchor, start_u);

            if (mapping == NULL) continue;
            if (!verify_isomorphism(n_g, adj_g, n_h, adj_h, mapping)) {
                free(mapping);
                continue;
            }
            if (mapping_exists(result, mapping)) {
                free(mapping);
                continue;
            }

            add_mapping(result, mapping);
            result->is_subgraph = true;

            printf("Found isomorphism %d (alt anchor G_%d -> H_%d)\n",
                   result->num_found, alt_anchor + 1, start_u + 1);
            print_single_mapping(result->num_found, result->mappings[result->num_found - 1], n_g, n_h, adj_g, adj_h);

            if (result->num_found >= n) {
                if (!interactive) { stop = true; } else if (!prompt_continue(
                    "Continue searching for more isomorphisms?")) { stop = true; }
            }
        }
    }

    free(sorted_g);
    free(sorted_h);
    return result;
}

// ============================================================================
// Utility Functions
// ============================================================================

void free_isomorphism_result(IsomorphismResult *result) {
    if (result) {
        for (int i = 0; i < result->num_found; i++) {
            free(result->mappings[i]);
        }
        free(result->mappings);
        free(result);
    }
}

void print_isomorphism_result(const IsomorphismResult *result) {
    printf("\n--- Isomorphism Result ---\n");
    printf("Subgraph isomorphism exists: %s\n", result->is_subgraph ? "YES" : "NO");
    printf("Number of isomorphisms found: %d\n", result->num_found);


    for (int i = 0; i < result->num_found; i++) {
        printf("\nMapping %d:\n", i + 1);
        for (int v = 0; v < result->n_g; v++) {
            printf("    G_%d -> H_%d\n", v + 1, result->mappings[i][v] + 1);
        }
    }
}
