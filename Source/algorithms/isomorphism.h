#ifndef AAC_ISOMORPHISM_H
#define AAC_ISOMORPHISM_H

#include <stdbool.h>

/**
 * Result structure for isomorphism finding
 */
typedef struct {
    int **mappings;      // Array of mappings, each mapping[i][v] = u means G_v -> H_u
    int num_found;       // Number of isomorphisms found
    int n_g;             // Size of G (for interpreting mappings)
    bool is_subgraph;    // True if at least one isomorphism exists (G is subgraph of H)
} IsomorphismResult;

/**
 * Find up to n distinct subgraph isomorphisms using exact algorithm (Bron-Kerbosch)
 *
 * @param n_g Number of vertices in G
 * @param adj_g Adjacency matrix of G (flattened)
 * @param n_h Number of vertices in H
 * @param adj_h Adjacency matrix of H (flattened)
 * @param n Maximum number of isomorphisms to find (0 = find all)
 * @return IsomorphismResult (caller must free with free_isomorphism_result)
 */
IsomorphismResult *find_isomorphisms_exact(int n_g, const int *adj_g,
                                            int n_h, const int *adj_h,
                                            int n);

/**
 * Find up to n distinct subgraph isomorphisms using heuristic (greedy clique)
 *
 * @param n_g Number of vertices in G
 * @param adj_g Adjacency matrix of G (flattened)
 * @param n_h Number of vertices in H
 * @param adj_h Adjacency matrix of H (flattened)
 * @param n Maximum number of isomorphisms to find
 * @return IsomorphismResult (caller must free with free_isomorphism_result)
 */
IsomorphismResult *find_isomorphisms_greedy(int n_g, const int *adj_g,
                                             int n_h, const int *adj_h,
                                             int n);

/**
 * Free memory allocated for isomorphism result
 */
void free_isomorphism_result(IsomorphismResult *result);

/**
 * Print isomorphism results
 */
void print_isomorphism_result(const IsomorphismResult *result);

/**
 * Check if two mappings are identical
 */
bool mappings_equal(const int *map1, const int *map2, int n_g);

#endif //AAC_ISOMORPHISM_H