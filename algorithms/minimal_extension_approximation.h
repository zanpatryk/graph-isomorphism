#ifndef AAC_MINIMAL_EXTENSION_APPROXIMATION_H
#define AAC_MINIMAL_EXTENSION_APPROXIMATION_H

#include "minimal_extension.h"  // Reuse ExtensionResult

/**
 * Find minimal extension for n isomorphisms using greedy heuristic
 *
 * This is faster than the exact algorithm but may not find the optimal solution.
 * Uses vertex-by-vertex greedy matching, iteratively updating H' after each mapping.
 *
 * @param n_g Number of vertices in G
 * @param adj_g Adjacency matrix of G (flattened)
 * @param n_h Number of vertices in H
 * @param adj_h Adjacency matrix of H (flattened)
 * @param n Number of distinct isomorphisms required
 * @return ExtensionResult (caller must free with free_extension_result)
 */
ExtensionResult *find_minimal_extension_greedy(int n_g, const int *adj_g,
                                                int n_h, const int *adj_h,
                                                int n, bool interactive);

/**
 * Legacy function for backwards compatibility
 */
void solve_approximate_extension(int n_g, const int *flat_adj_g, int n_h, const int *flat_adj_h);

#endif //AAC_MINIMAL_EXTENSION_APPROXIMATION_H