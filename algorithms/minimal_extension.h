#ifndef AAC_MINIMAL_EXTENSION_H
#define AAC_MINIMAL_EXTENSION_H

/**
 * Result structure for minimal extension
 */
typedef struct {
    int **mappings;           // Array of n mappings
    int num_mappings;         // Number of mappings found
    int *extended_adj_h;      // Extended adjacency matrix H' (flattened)
    int n_h;                  // Size of H (for interpreting extended_adj_h)
    int n_g;                  // Size of G (for interpreting mappings)
    int total_edges_added;    // Total number of edges added to H
} ExtensionResult;

/**
 * Find minimal extension for n isomorphisms using exact algorithm (backtracking)
 *
 * Note: For n > 1, this uses an iterative approach:
 * - Find best mapping for current H'
 * - Add required edges to H'
 * - Repeat n times
 *
 * This is NOT globally optimal for n > 1, but is tractable.
 *
 * @param n_g Number of vertices in G
 * @param adj_g Adjacency matrix of G (flattened)
 * @param n_h Number of vertices in H
 * @param adj_h Adjacency matrix of H (flattened)
 * @param n Number of distinct isomorphisms required
 * @return ExtensionResult (caller must free with free_extension_result)
 */
ExtensionResult *find_minimal_extension_exact(int n_g, const int *adj_g,
                                               int n_h, const int *adj_h,
                                               int n);

/**
 * Free memory allocated for extension result
 */
void free_extension_result(ExtensionResult *result);

/**
 * Print extension results
 */
void print_extension_result(const ExtensionResult *result, const int *adj_g);

/**
 * Legacy function for backwards compatibility
 */
void solve_minimal_extension(int n_g, const int *flat_adj_g, int n_h, const int *flat_adj_h);


#endif //AAC_MINIMAL_EXTENSION_H