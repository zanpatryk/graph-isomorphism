#ifndef AAC_PRODUCT_GRAPH_H
#define AAC_PRODUCT_GRAPH_H

#include <stdbool.h>

/**
 * Modular Product Graph for Subgraph Isomorphism
 *
 * A vertex (v, u) in the product graph represents a potential mapping v -> u
 * An edge between (v1, u1) and (v2, u2) exists iff:
 *   - v1 != v2 and u1 != u2 (injectivity)
 *   - mult_G(v1, v2) <= mult_H(u1, u2) (forward edge preservation)
 *   - mult_G(v2, v1) <= mult_H(u2, u1) (backward edge preservation)
 */

typedef struct {
    int v; // Vertex in G
    int u; // Vertex in H
} ProductVertex;

typedef struct {
    ProductVertex *vertices; // Array of product graph vertices
    int num_vertices; // Number of vertices in product graph
    bool *adj_matrix; // Adjacency matrix (flattened, num_vertices x num_vertices)
    int *degree; // Degree of each vertex in product graph

    // Original graph info (for reference)
    int n_g; // |V(G)|
    int n_h; // |V(H)|
} ProductGraph;

/**
 * Build the modular product graph P = G ⊗ H
 *
 * @param n_g Number of vertices in G
 * @param adj_g Adjacency matrix of G (flattened, n_g x n_g)
 * @param n_h Number of vertices in H
 * @param adj_h Adjacency matrix of H (flattened, n_h x n_h)
 * @return Pointer to ProductGraph (caller must free with free_product_graph)
 */
ProductGraph *build_product_graph(int n_g, const int *adj_g, int n_h, const int *adj_h);

/**
 * Free memory allocated for product graph
 */
void free_product_graph(ProductGraph *pg);

/**
 * Print product graph info (for debugging)
 */
void print_product_graph(const ProductGraph *pg);

/**
 * Check if two product vertices are adjacent
 */
bool product_graph_adjacent(const ProductGraph *pg, int idx1, int idx2);

/**
 * Get degree of a vertex in the product graph
 */
int product_graph_degree(const ProductGraph *pg, int idx);

#endif //AAC_PRODUCT_GRAPH_H
