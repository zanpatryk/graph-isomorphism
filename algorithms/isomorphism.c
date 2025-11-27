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
// Exact Algorithm: Bron-Kerbosch with pivoting
// ============================================================================

typedef struct {
    const ProductGraph *pg;
    IsomorphismResult *result;
    int target_size;
    int max_to_find;
} BKContext;

// Recursive Bron-Kerbosch
static void bron_kerbosch(BKContext *ctx,
                          int *R, int r_size,
                          int *P, int p_size,
                          int *X, int x_size) {
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
    if (r_size + p_size < ctx->target_size) {
        return;
    }

    // Base case: no more candidates
    if (p_size == 0) {
        return;
    }

    // Choose pivot: vertex in P ∪ X with most neighbors in P
    int pivot = -1;
    int max_neighbors = -1;

    for (int i = 0; i < p_size; i++) {
        int v = P[i];
        int neighbors = 0;
        for (int j = 0; j < p_size; j++) {
            if (product_graph_adjacent(ctx->pg, v, P[j])) neighbors++;
        }
        if (neighbors > max_neighbors) {
            max_neighbors = neighbors;
            pivot = v;
        }
    }
    for (int i = 0; i < x_size; i++) {
        int v = X[i];
        int neighbors = 0;
        for (int j = 0; j < p_size; j++) {
            if (product_graph_adjacent(ctx->pg, v, P[j])) neighbors++;
        }
        if (neighbors > max_neighbors) {
            max_neighbors = neighbors;
            pivot = v;
        }
    }

    // Iterate over P \ N(pivot)
    int *P_copy = (int *)malloc(p_size * sizeof(int));
    memcpy(P_copy, P, p_size * sizeof(int));
    int p_copy_size = p_size;

    for (int idx = 0; idx < p_copy_size; idx++) {
        int v = P_copy[idx];

        // Skip if v is neighbor of pivot
        if (pivot >= 0 && product_graph_adjacent(ctx->pg, pivot, v)) {
            continue;
        }

        // Check injectivity with current clique
        bool valid = true;
        int v_g = ctx->pg->vertices[v].v;
        int v_h = ctx->pg->vertices[v].u;
        for (int i = 0; i < r_size && valid; i++) {
            int r_g = ctx->pg->vertices[R[i]].v;
            int r_h = ctx->pg->vertices[R[i]].u;
            if (v_g == r_g || v_h == r_h) valid = false;
        }
        if (!valid) continue;

        // Build new sets
        int *new_R = (int *)malloc((r_size + 1) * sizeof(int));
        memcpy(new_R, R, r_size * sizeof(int));
        new_R[r_size] = v;

        int *new_P = (int *)malloc(p_size * sizeof(int));
        int new_p_size = 0;
        for (int i = 0; i < p_size; i++) {
            if (P[i] != v && product_graph_adjacent(ctx->pg, v, P[i])) {
                // Also check injectivity
                int p_g = ctx->pg->vertices[P[i]].v;
                int p_h = ctx->pg->vertices[P[i]].u;
                bool inj_ok = (p_g != v_g && p_h != v_h);
                for (int j = 0; j < r_size && inj_ok; j++) {
                    int r_g = ctx->pg->vertices[R[j]].v;
                    int r_h = ctx->pg->vertices[R[j]].u;
                    if (p_g == r_g || p_h == r_h) inj_ok = false;
                }
                if (inj_ok) {
                    new_P[new_p_size++] = P[i];
                }
            }
        }

        int *new_X = (int *)malloc(x_size * sizeof(int));
        int new_x_size = 0;
        for (int i = 0; i < x_size; i++) {
            if (product_graph_adjacent(ctx->pg, v, X[i])) {
                new_X[new_x_size++] = X[i];
            }
        }

        // Recurse
        bron_kerbosch(ctx, new_R, r_size + 1, new_P, new_p_size, new_X, new_x_size);

        free(new_R);
        free(new_P);
        free(new_X);

        // Move v from P to X
        for (int i = 0; i < p_size; i++) {
            if (P[i] == v) {
                P[i] = P[p_size - 1];
                p_size--;
                break;
            }
        }

        // Add v to X
        X[x_size++] = v;

        // Check if we've found enough
        if (ctx->max_to_find > 0 && ctx->result->num_found >= ctx->max_to_find) {
            break;
        }
    }

    free(P_copy);
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
    if (!pg || pg->num_vertices == 0) {
        printf("Product graph is empty. No isomorphism possible.\n");
        free_product_graph(pg);
        return result;
    }

    // Initialize Bron-Kerbosch
    BKContext ctx = {
        .pg = pg,
        .result = result,
        .target_size = n_g,
        .max_to_find = n
    };

    int *R = (int *)malloc(pg->num_vertices * sizeof(int));
    int *P = (int *)malloc(pg->num_vertices * sizeof(int));
    int *X = (int *)malloc(pg->num_vertices * sizeof(int));

    for (int i = 0; i < pg->num_vertices; i++) {
        P[i] = i;
    }

    bron_kerbosch(&ctx, R, 0, P, pg->num_vertices, X, 0);

    free(R);
    free(P);
    free(X);
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
    if (!pg || pg->num_vertices == 0) {
        printf("Product graph is empty. No isomorphism possible.\n");
        free_product_graph(pg);
        return result;
    }

    bool *excluded = (bool *)calloc(pg->num_vertices, sizeof(bool));

    for (int k = 0; k < n && k < MAX_ISOMORPHISMS; k++) {
        int clique_size;
        int *clique = greedy_clique(pg, n_g, excluded, &clique_size);

        if (clique_size < n_g) {
            // Couldn't find a complete clique
            free(clique);
            break;
        }

        int *mapping = extract_mapping(pg, clique, clique_size);

        // Check if this mapping is distinct
        if (!mapping_exists(result, mapping)) {
            add_mapping(result, mapping);
            result->is_subgraph = true;

            // Exclude one vertex from this clique to find different one next time
            if (k < n - 1) {
                for (int i = 0; i < clique_size; i++) {
                    // Try excluding each vertex until we find a new mapping
                    excluded[clique[i]] = true;

                    int test_size;
                    int *test_clique = greedy_clique(pg, n_g, excluded, &test_size);

                    if (test_size == n_g) {
                        int *test_mapping = extract_mapping(pg, test_clique, test_size);
                        if (!mapping_exists(result, test_mapping)) {
                            // This exclusion leads to a new mapping, keep it
                            free(test_mapping);
                            free(test_clique);
                            break;
                        }
                        free(test_mapping);
                    }

                    // Undo exclusion, try next vertex
                    excluded[clique[i]] = false;
                    free(test_clique);
                }
            }
        } else {
            free(mapping);
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