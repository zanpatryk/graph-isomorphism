#include "isomorphism.h"
#include "product_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ISOMORPHISMS 1000

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
    int *mapping = (int *)malloc(pg->n_g * sizeof(int));
    for (int i = 0; i < pg->n_g; i++) {
        mapping[i] = -1;  // Initialize as unmapped
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

// ============================================================================
// Exact Algorithm: Bron-Kerbosch with pivoting (simplified & fixed)
// ============================================================================

typedef struct {
    const ProductGraph *pg;
    IsomorphismResult *result;
    int target_size;
    int max_to_find;
    int max_pg_vertices;  // For safe allocation
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
    // Check if we've found enough
    if (ctx->max_to_find > 0 && ctx->result->num_found >= ctx->max_to_find) {
        return;
    }

    // Found a clique of target size
    if (r_size == ctx->target_size) {
        int *mapping = extract_mapping(ctx->pg, R, r_size);
        if (!mapping_exists(ctx->result, mapping)) {
            add_mapping(ctx->result, mapping);
            ctx->result->is_subgraph = true;
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
        int *new_candidates = (int *)malloc(num_candidates * sizeof(int));
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
                                            int n) {
    IsomorphismResult *result = (IsomorphismResult *)malloc(sizeof(IsomorphismResult));
    result->mappings = (int **)malloc(MAX_ISOMORPHISMS * sizeof(int *));
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
        int *empty_mapping = (int *)calloc(1, sizeof(int));
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
        .max_to_find = n,
        .max_pg_vertices = pg->num_vertices
    };

    // Allocate working arrays
    int *R = (int *)malloc(pg->num_vertices * sizeof(int));
    int *candidates = (int *)malloc(pg->num_vertices * sizeof(int));

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
// Heuristic Algorithm: Greedy Clique
// ============================================================================

// Comparator for sorting by degree (descending)
typedef struct {
    int idx;
    int degree;
} VertexDegree;

static int compare_degree_desc(const void *a, const void *b) {
    const VertexDegree *va = (const VertexDegree *)a;
    const VertexDegree *vb = (const VertexDegree *)b;
    if (vb->degree != va->degree) {
        return vb->degree - va->degree;
    }
    return va->idx - vb->idx;  // Tie-break by index
}

// Find a greedy clique, optionally excluding certain vertices
static int *greedy_clique(const ProductGraph *pg, int target_size,
                          const bool *excluded, int *out_size) {
    int *clique = (int *)malloc(target_size * sizeof(int));
    int clique_size = 0;

    // Sort vertices by degree
    VertexDegree *sorted = (VertexDegree *)malloc(pg->num_vertices * sizeof(VertexDegree));
    for (int i = 0; i < pg->num_vertices; i++) {
        sorted[i].idx = i;
        sorted[i].degree = pg->degree[i];
    }
    qsort(sorted, pg->num_vertices, sizeof(VertexDegree), compare_degree_desc);

    // Track which G and H vertices are used (for injectivity)
    bool *used_g = (bool *)calloc(pg->n_g, sizeof(bool));
    bool *used_h = (bool *)calloc(pg->n_h, sizeof(bool));

    for (int i = 0; i < pg->num_vertices && clique_size < target_size; i++) {
        int v = sorted[i].idx;

        // Skip if excluded
        if (excluded && excluded[v]) continue;

        int v_g = pg->vertices[v].v;
        int v_h = pg->vertices[v].u;

        // Check injectivity
        if (used_g[v_g] || used_h[v_h]) continue;

        // Check adjacency with all current clique members
        bool compatible = true;
        for (int j = 0; j < clique_size && compatible; j++) {
            if (!product_graph_adjacent(pg, v, clique[j])) {
                compatible = false;
            }
        }

        if (compatible) {
            clique[clique_size++] = v;
            used_g[v_g] = true;
            used_h[v_h] = true;
        }
    }

    free(sorted);
    free(used_g);
    free(used_h);

    *out_size = clique_size;
    return clique;
}

IsomorphismResult *find_isomorphisms_greedy(int n_g, const int *adj_g,
                                             int n_h, const int *adj_h,
                                             int n) {
    IsomorphismResult *result = (IsomorphismResult *)malloc(sizeof(IsomorphismResult));
    result->mappings = (int **)malloc(MAX_ISOMORPHISMS * sizeof(int *));
    result->num_found = 0;
    result->n_g = n_g;
    result->is_subgraph = false;

    // Edge case
    if (n_g > n_h) {
        printf("G has more vertices than H. No isomorphism possible.\n");
        return result;
    }

    if (n_g == 0) {
        result->is_subgraph = true;
        int *empty_mapping = (int *)calloc(1, sizeof(int));
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

    bool *excluded = (bool *)calloc(pg->num_vertices, sizeof(bool));

    for (int k = 0; k < n && k < MAX_ISOMORPHISMS; k++) {
        int clique_size;
        int *clique = greedy_clique(pg, n_g, excluded, &clique_size);

        if (clique_size < n_g) {
            // Couldn't find a complete clique
            free(clique);

            // Try excluding different vertices
            bool found_alternative = false;
            if (k > 0 && result->num_found > 0) {
                // Reset exclusions and try different strategy
                memset(excluded, 0, pg->num_vertices * sizeof(bool));

                // Exclude all vertices from previous mappings
                for (int m = 0; m < result->num_found; m++) {
                    int *prev_map = result->mappings[m];
                    for (int i = 0; i < pg->num_vertices; i++) {
                        int v_g = pg->vertices[i].v;
                        int v_h = pg->vertices[i].u;
                        if (prev_map[v_g] == v_h) {
                            excluded[i] = true;
                        }
                    }
                }

                clique = greedy_clique(pg, n_g, excluded, &clique_size);
                if (clique_size == n_g) {
                    found_alternative = true;
                } else {
                    free(clique);
                }
            }

            if (!found_alternative) {
                break;
            }
        }

        int *mapping = extract_mapping(pg, clique, clique_size);

        // Check if this mapping is distinct
        if (!mapping_exists(result, mapping)) {
            add_mapping(result, mapping);
            result->is_subgraph = true;

            // Mark vertices from this clique for potential exclusion
            for (int i = 0; i < clique_size; i++) {
                excluded[clique[i]] = true;
            }
        } else {
            free(mapping);
            // Exclude more vertices to find different mapping
            for (int i = 0; i < clique_size; i++) {
                excluded[clique[i]] = true;
            }
        }

        free(clique);
    }

    free(excluded);
    free_product_graph(pg);

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