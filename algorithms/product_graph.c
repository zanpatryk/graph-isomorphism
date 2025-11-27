#include "product_graph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper: Get adjacency value from flattened matrix
static inline int get_adj(const int *adj, int n, int i, int j) {
    return adj[i * n + j];
}

// Helper: Calculate in-degree for vertex v in graph with adjacency matrix adj
static int calc_in_degree(const int *adj, int n, int v) {
    int deg = 0;
    for (int u = 0; u < n; u++) {
        deg += get_adj(adj, n, u, v);
    }
    return deg;
}

// Helper: Calculate out-degree for vertex v in graph with adjacency matrix adj
static int calc_out_degree(const int *adj, int n, int v) {
    int deg = 0;
    for (int u = 0; u < n; u++) {
        deg += get_adj(adj, n, v, u);
    }
    return deg;
}

ProductGraph *build_product_graph(int n_g, const int *adj_g, int n_h, const int *adj_h) {
    ProductGraph *pg = (ProductGraph *)malloc(sizeof(ProductGraph));
    if (!pg) return NULL;

    pg->n_g = n_g;
    pg->n_h = n_h;

    // Phase 1: Count compatible vertices and allocate
    // A pair (v, u) is compatible if deg+_G(v) <= deg+_H(u) and deg-_G(v) <= deg-_H(u)

    // Pre-calculate degrees for all vertices
    int *in_deg_g = (int *)malloc(n_g * sizeof(int));
    int *out_deg_g = (int *)malloc(n_g * sizeof(int));
    int *in_deg_h = (int *)malloc(n_h * sizeof(int));
    int *out_deg_h = (int *)malloc(n_h * sizeof(int));

    for (int v = 0; v < n_g; v++) {
        in_deg_g[v] = calc_in_degree(adj_g, n_g, v);
        out_deg_g[v] = calc_out_degree(adj_g, n_g, v);
    }
    for (int u = 0; u < n_h; u++) {
        in_deg_h[u] = calc_in_degree(adj_h, n_h, u);
        out_deg_h[u] = calc_out_degree(adj_h, n_h, u);
    }

    // Count compatible pairs
    int max_vertices = n_g * n_h;  // Upper bound
    ProductVertex *temp_vertices = (ProductVertex *)malloc(max_vertices * sizeof(ProductVertex));
    int count = 0;

    for (int v = 0; v < n_g; v++) {
        for (int u = 0; u < n_h; u++) {
            // Check degree constraints
            if (out_deg_g[v] <= out_deg_h[u] && in_deg_g[v] <= in_deg_h[u]) {
                temp_vertices[count].v = v;
                temp_vertices[count].u = u;
                count++;
            }
        }
    }

    pg->num_vertices = count;
    pg->vertices = (ProductVertex *)malloc(count * sizeof(ProductVertex));
    memcpy(pg->vertices, temp_vertices, count * sizeof(ProductVertex));
    free(temp_vertices);

    // Phase 2: Build adjacency matrix for product graph
    pg->adj_matrix = (bool *)calloc((size_t)count * count, sizeof(bool));
    pg->degree = (int *)calloc(count, sizeof(int));

    for (int i = 0; i < count; i++) {
        int v1 = pg->vertices[i].v;
        int u1 = pg->vertices[i].u;

        for (int j = i + 1; j < count; j++) {
            int v2 = pg->vertices[j].v;
            int u2 = pg->vertices[j].u;

            // Check injectivity: v1 != v2 and u1 != u2
            if (v1 == v2 || u1 == u2) continue;

            // Check edge preservation (both directions for directed graph)
            int g_forward = get_adj(adj_g, n_g, v1, v2);
            int h_forward = get_adj(adj_h, n_h, u1, u2);
            int g_backward = get_adj(adj_g, n_g, v2, v1);
            int h_backward = get_adj(adj_h, n_h, u2, u1);

            if (g_forward <= h_forward && g_backward <= h_backward) {
                // Edge exists in product graph (undirected)
                pg->adj_matrix[i * count + j] = true;
                pg->adj_matrix[j * count + i] = true;
                pg->degree[i]++;
                pg->degree[j]++;
            }
        }
    }

    // Cleanup
    free(in_deg_g);
    free(out_deg_g);
    free(in_deg_h);
    free(out_deg_h);

    return pg;
}

void free_product_graph(ProductGraph *pg) {
    if (pg) {
        free(pg->vertices);
        free(pg->adj_matrix);
        free(pg->degree);
        free(pg);
    }
}

void print_product_graph(const ProductGraph *pg) {
    printf("Product Graph: %d vertices\n", pg->num_vertices);
    printf("Vertices (v_G, u_H):\n");
    for (int i = 0; i < pg->num_vertices; i++) {
        printf("  [%d]: (G_%d, H_%d) degree=%d\n",
               i, pg->vertices[i].v + 1, pg->vertices[i].u + 1, pg->degree[i]);
    }

    printf("Edges:\n");
    int edge_count = 0;
    for (int i = 0; i < pg->num_vertices; i++) {
        for (int j = i + 1; j < pg->num_vertices; j++) {
            if (pg->adj_matrix[i * pg->num_vertices + j]) {
                printf("  [%d]-[%d]: (G_%d,H_%d)-(G_%d,H_%d)\n",
                       i, j,
                       pg->vertices[i].v + 1, pg->vertices[i].u + 1,
                       pg->vertices[j].v + 1, pg->vertices[j].u + 1);
                edge_count++;
            }
        }
    }
    printf("Total edges: %d\n", edge_count);
}

bool product_graph_adjacent(const ProductGraph *pg, int idx1, int idx2) {
    if (idx1 < 0 || idx1 >= pg->num_vertices || idx2 < 0 || idx2 >= pg->num_vertices) {
        return false;
    }
    return pg->adj_matrix[idx1 * pg->num_vertices + idx2];
}

int product_graph_degree(const ProductGraph *pg, int idx) {
    if (idx < 0 || idx >= pg->num_vertices) {
        return 0;
    }
    return pg->degree[idx];
}