#define _POSIX_C_SOURCE 200809L // Required for getline and strdup
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

// ==========================================
// PART 1: The Backtracking Algorithm Logic
// ==========================================

#define MAX_VERTICES 20

// --- Algorithm Globals ---
int min_edges_to_add = INT_MAX;
int best_mapping[MAX_VERTICES];

int G_adj[MAX_VERTICES][MAX_VERTICES];
int H_adj[MAX_VERTICES][MAX_VERTICES];
int n_g_global;
int n_h_global;

/**
 * Calculates cost for the current mapping.
 */
int calculate_extension_cost(int *current_mapping) {
    int current_edges_to_add = 0;
    for (int i = 0; i < n_g_global; i++) {
        for (int j = 0; j < n_g_global; j++) {
            if (G_adj[i][j] > 0) {
                int u_mapped = current_mapping[i];
                int v_mapped = current_mapping[j];
                int g_mult = G_adj[i][j];
                int h_mult = H_adj[u_mapped][v_mapped];

                if (h_mult < g_mult) {
                    current_edges_to_add += (g_mult - h_mult);
                }
            }
        }
    }
    return current_edges_to_add;
}

/**
 * Recursive Backtracking with STRICT 1-to-1 Constraint
 */
void backtrack_extend(int v_index, int *mapping, bool *used_u) {
    // --- Base Case: All vertices in G are mapped ---
    if (v_index == n_g_global) {
        // NOW it is safe to calculate cost because 'mapping' is fully filled
        int cost = calculate_extension_cost(mapping);
        if (cost < min_edges_to_add) {
            min_edges_to_add = cost;
            for (int i = 0; i < n_g_global; i++) {
                best_mapping[i] = mapping[i];
            }
        }
        return;
    }

    // --- Recursive Step ---
    for (int u = 0; u < n_h_global; u++) {
        // Strict 1-to-1 check
        if (!used_u[u]) {
            mapping[v_index] = u;
            used_u[u] = true;

            // REMOVED THE PRUNING BLOCK HERE TO PREVENT SEGFAULT
            backtrack_extend(v_index + 1, mapping, used_u);

            used_u[u] = false; // Backtrack
        }
    }
}

void print_solution_details() {
    printf("\n--- Minimal Extension Result ---\n");
    printf("Minimal number of added edges: %d\n", min_edges_to_add);

    printf("\n1. Vertex Mapping (G -> H):\n");
    for (int i = 0; i < n_g_global; i++) {
        printf("   G_%d -> H_%d\n", i + 1, best_mapping[i] + 1);
    }

    printf("\n2. Edges to Add to H:\n");
    if (min_edges_to_add == 0) {
        printf("   None. G is already a subgraph of H.\n");
    } else {
        for (int i = 0; i < n_g_global; i++) {
            for (int j = 0; j < n_g_global; j++) {
                if (G_adj[i][j] > 0) {
                    int u = best_mapping[i];
                    int v = best_mapping[j];
                    int required = G_adj[i][j];
                    int existing = H_adj[u][v];

                    if (existing < required) {
                        int missing = required - existing;
                        printf("   Add %d edge(s): H_%d -> H_%d  (for G_%d -> G_%d)\n",
                               missing, u + 1, v + 1, i + 1, j + 1);
                    }
                }
            }
        }
    }
}

// ==========================================
// PART 2: File Parsing Logic (Your Code)
// ==========================================

// Remove leading and trailing whitespace
static char *trim_inplace(char *s) {
    if (!s) return s;
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) s[--len] = '\0';
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end-1))) *(--end) = '\0';
    if (start != s) memmove(s, start, strlen(start) + 1);
    return s;
}

static int parse_vertex_index(const char *token) {
    const char *p = token;
    while (*p && !isdigit((unsigned char)*p)) p++;
    if (!*p) return -1;
    int v = atoi(p);
    return v - 1;
}

static int is_header_line(const char *s) {
    if (!s || s[0] == '\0') return 0;
    return strchr(s, '-') == NULL;
}

static int read_all_lines(const char *path, char ***out_lines, size_t *out_n) {
    FILE *f = fopen(path, "r");
    if (!f) return -1;
    char *line = NULL;
    size_t len = 0;
    ssize_t r;
    size_t cap = 128, cnt = 0;
    char **lines = malloc(cap * sizeof(char*));
    if (!lines) { fclose(f); return -1; }

    while ((r = getline(&line, &len, f)) != -1) {
        if (cnt >= cap) {
            cap *= 2;
            char **tmp = realloc(lines, cap * sizeof(char*));
            if (!tmp) { while (cnt--) free(lines[cnt]); free(lines); free(line); fclose(f); return -1; }
            lines = tmp;
        }
        trim_inplace(line);
        lines[cnt++] = strdup(line);
    }
    free(line);
    fclose(f);
    *out_lines = lines;
    *out_n = cnt;
    return 0;
}

static int parse_graph_at(char **lines, size_t nlines, size_t header_idx, int *n_out, int **adj_out) {
    if (header_idx >= nlines) return -1;
    // const char *name = lines[header_idx]; // Unused variable
    size_t i = header_idx + 1;
    while (i < nlines && lines[i][0] == '\0') ++i;
    if (i >= nlines) return -1;
    int n = atoi(lines[i]);
    if (n <= 0) return -1;
    int *adj = calloc((size_t)n * (size_t)n, sizeof(int));
    if (!adj) return -1;
    i++;
    for (; i < nlines; ++i) {
        if (lines[i][0] == '\0') continue;
        if (is_header_line(lines[i])) break;
        char *copy = strdup(lines[i]);
        char *lhs = strtok(copy, "-");
        char *rhs = strtok(NULL, "-");
        if (!lhs || !rhs) { free(copy); continue; }
        trim_inplace(lhs); trim_inplace(rhs);
        int u = parse_vertex_index(lhs);
        int v = parse_vertex_index(rhs);
        if (u < 0 || v < 0 || u >= n || v >= n) { free(copy); continue; }
        adj[u * n + v] += 1;
        free(copy);
    }
    *n_out = n;
    *adj_out = adj;
    return 0;
}

static int find_header_indices(char **lines, size_t nlines, ssize_t *out_g, ssize_t *out_h) {
    *out_g = -1; *out_h = -1;
    for (size_t i = 0; i < nlines; ++i) {
        if (lines[i][0] == '\0') continue;
        size_t L = strlen(lines[i]);
        if (L == 1) {
            if (lines[i][0] == 'g') *out_g = (ssize_t)i;
            else if (lines[i][0] == 'h') *out_h = (ssize_t)i;
        }
    }
    return 0;
}

int load_graphs(const char *path, int *n_g, int **adj_g, int *n_h, int **adj_h){
    char **lines = NULL;
    size_t nlines = 0;

    if(read_all_lines(path, &lines, &nlines) != 0){
        fprintf(stderr, "Error: Failed to read file: %s\n", path);
        return 1;
    }

    ssize_t idx_g = -1, idx_h = -1;
    find_header_indices(lines, nlines, &idx_g, &idx_h);

    if(idx_g < 0 || idx_h < 0){
        fprintf(stderr, "Error: Both headers 'g' and 'h' must be present in %s\n", path);
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 2;
    }

    if (parse_graph_at(lines, nlines, (size_t)idx_g, n_g, adj_g) != 0) {
        fprintf(stderr, "Error: Failed parsing graph 'g'\n");
        // cleanup
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 3;
    }
    if (parse_graph_at(lines, nlines, (size_t)idx_h, n_h, adj_h) != 0) {
        fprintf(stderr, "Error: Failed parsing graph 'h'\n");
        free(*adj_g);
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 4;
    }

    for (size_t j = 0; j < nlines; ++j) free(lines[j]);
    free(lines);
    return 0;
}

// ==========================================
// PART 3: The Bridge Function
// ==========================================

void solve_minimal_extension(int n_g, int *flat_adj_g, int n_h, int *flat_adj_h) {
    printf("Running Minimal Extension Algorithm...\n");

    // 1. Validation
    if (n_g > MAX_VERTICES || n_h > MAX_VERTICES) {
        fprintf(stderr, "Error: Graph size exceeds hardcoded limit of %d vertices.\n", MAX_VERTICES);
        return;
    }
    if (n_g > n_h) {
        fprintf(stderr, "Error: G (%d vertices) is larger than H (%d vertices). Subgraph isomorphism impossible.\n", n_g, n_h);
        return;
    }

    // 2. Copy data from flat parser arrays to algorithm's 2D globals
    n_g_global = n_g;
    n_h_global = n_h;

    // Clear globals first
    memset(G_adj, 0, sizeof(G_adj));
    memset(H_adj, 0, sizeof(H_adj));

    // Copy G (flat_adj_g uses row*n + col)
    for (int r = 0; r < n_g; r++) {
        for (int c = 0; c < n_g; c++) {
            G_adj[r][c] = flat_adj_g[r * n_g + c];
        }
    }
    // Copy H
    for (int r = 0; r < n_h; r++) {
        for (int c = 0; c < n_h; c++) {
            H_adj[r][c] = flat_adj_h[r * n_h + c];
        }
    }

    // 3. Initialize Algorithm Variables
    int total_edges_G = 0;
    for(int i=0; i<n_g; i++)
        for(int j=0; j<n_g; j++)
            total_edges_G += G_adj[i][j];

    min_edges_to_add = total_edges_G; // Worst case init

    int *partial_mapping = (int*)malloc(n_g * sizeof(int));
    bool *used_u = (bool*)calloc(n_h, sizeof(bool));

    // 4. Run Backtracking
    backtrack_extend(0, partial_mapping, used_u);

    // 5. Print Results
    print_solution_details();

    // Cleanup local allocations
    free(partial_mapping);
    free(used_u);
}

// ==========================================
// PART 4: Main (CLI Entry Point)
// ==========================================

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <function_name> <path_to_graph_data>\n", argv[0]);
        fprintf(stderr, "Available functions:\n");
        fprintf(stderr, "  minimal_extension\n");
        return 1;
    }

    const char *function_name = argv[1];
    const char *file_path = argv[2];

    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;

    // Load Graph Data
    if (load_graphs(file_path, &n_g, &adj_g, &n_h, &adj_h) != 0) {
        return 1; // load_graphs prints its own errors
    }

    // Dispatch Function
    if (strcmp(function_name, "minimal_extension") == 0) {
        solve_minimal_extension(n_g, adj_g, n_h, adj_h);
    } else {
        fprintf(stderr, "Error: Unknown function '%s'\n", function_name);
        free(adj_g);
        free(adj_h);
        return 1;
    }

    // Cleanup Parser Data
    free(adj_g);
    free(adj_h);

    return 0;
}