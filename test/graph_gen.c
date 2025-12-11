#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Helper: Memory Management ---
int** create_matrix(int size) {
    int** matrix = (int**)malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++) {
        matrix[i] = (int*)calloc(size, sizeof(int)); // calloc initializes to 0
    }
    return matrix;
}

void free_matrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// --- Helper: Sorting for indices ---
int compare_ints(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

// --- Logic: Random Generation ---
int** generate_random_multigraph(int size, double density, int max_multiedges) {
    int** matrix = create_matrix(size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            // Random float between 0.0 and 1.0
            double r = (double)rand() / (double)RAND_MAX;
            if (r < density) {
                matrix[i][j] = (rand() % max_multiedges) + 1;
            }
        }
    }
    return matrix;
}

// --- Logic: Permutation ---
int** permute_matrix(int** matrix, int n) {
    int* p = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) p[i] = i;

    // Fisher-Yates Shuffle
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = p[i];
        p[i] = p[j];
        p[j] = temp;
    }

    int** new_matrix = create_matrix(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            new_matrix[i][j] = matrix[p[i]][p[j]];
        }
    }
    free(p);
    return new_matrix;
}

// --- Logic: Subgraph Extraction ---
int** extract_subgraph(int** H_matrix, int H_size, int sub_size) {
    if (sub_size > H_size) {
        fprintf(stderr, "Error: Subgraph size cannot be larger than original graph.\n");
        exit(1);
    }

    // select random unique indices
    int* all_indices = (int*)malloc(H_size * sizeof(int));
    for (int i = 0; i < H_size; i++) all_indices[i] = i;

    // Shuffle and pick first sub_size elements (mimics random.sample)
    for (int i = H_size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = all_indices[i];
        all_indices[i] = all_indices[j];
        all_indices[j] = temp;
    }

    // Sort the chosen indices to maintain relative order (optional, but matches Python sort)
    qsort(all_indices, sub_size, sizeof(int), compare_ints);

    int** G_matrix = create_matrix(sub_size);
    for (int i = 0; i < sub_size; i++) {
        for (int j = 0; j < sub_size; j++) {
            G_matrix[i][j] = H_matrix[all_indices[i]][all_indices[j]];
        }
    }

    free(all_indices);
    return G_matrix;
}

int get_max_weight(int** matrix, int size) {
    int max_val = 0;
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (matrix[i][j] > max_val) max_val = matrix[i][j];
        }
    }
    return max_val;
}

// --- Main ---
int main(int argc, char* argv[]) {
    srand(time(NULL)); // Seed RNG

    // Quick and dirty arg parsing
    // Usage: ./graph_gen <g_size> <h_size> [--not-subgraph] --output <file>
    if (argc < 5) {
        printf("Usage: %s <g_size> <h_size> [--not-subgraph] --output <file>\n", argv[0]);
        return 1;
    }

    int g_size = atoi(argv[1]);
    int h_size = atoi(argv[2]);
    int is_subgraph = 1; // Default True
    char* output_file = NULL;

    // Loop through remaining args
    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--not-subgraph") == 0) {
            is_subgraph = 0;
        } else if (strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[i + 1];
                i++; // Skip next arg
            }
        }
    }

    if (!output_file) {
        fprintf(stderr, "Error: --output filename required.\n");
        return 1;
    }

    // 1. Generate H
    int** H_base = generate_random_multigraph(h_size, 0.3, 4);

    // 2. Extract G
    int** G = extract_subgraph(H_base, h_size, g_size);
    int** H_final;

    // 3. Apply Logic
    if (is_subgraph) {
        H_final = permute_matrix(H_base, h_size);
    } else {
        int max_weight_in_H = get_max_weight(H_base, h_size);

        // Find existing edges
        int r = -1, c = -1;
        // Simple search for first non-zero edge, or random if empty
        int found = 0;
        int attempts = 0;

        // Try to find an existing edge to boost (up to 100 tries to keep it fast)
        while (attempts < 100 && !found) {
            int tr = rand() % g_size;
            int tc = rand() % g_size;
            if (G[tr][tc] > 0) {
                r = tr; c = tc;
                found = 1;
            }
            attempts++;
        }

        // Fallback if graph is empty or search failed
        if (!found) {
            r = rand() % g_size;
            c = rand() % g_size;
        }

        G[r][c] = max_weight_in_H + 1;
        H_final = permute_matrix(H_base, h_size);
    }

    // 4. Write Output
    FILE* f = fopen(output_file, "w");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    // Write G
    fprintf(f, "%d\n", g_size);
    for (int i = 0; i < g_size; i++) {
        for (int j = 0; j < g_size; j++) {
            fprintf(f, "%d%s", G[i][j], (j == g_size - 1) ? "" : " ");
        }
        fprintf(f, "\n");
    }

    // Write H
    fprintf(f, "%d\n", h_size);
    for (int i = 0; i < h_size; i++) {
        for (int j = 0; j < h_size; j++) {
            fprintf(f, "%d%s", H_final[i][j], (j == h_size - 1) ? "" : " ");
        }
        fprintf(f, "\n");
    }

    fclose(f);

    // Cleanup memory
    free_matrix(H_base, h_size);
    free_matrix(H_final, h_size);
    free_matrix(G, g_size);

    return 0;
}