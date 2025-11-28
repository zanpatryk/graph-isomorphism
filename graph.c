#include "graph.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// --- Private Helper Functions ---

static int read_adjacency_matrix(FILE *f, int n, int *adj) {
    char buffer[4096];
    for (int i = 0; i < n; ++i) {
        if (!fgets(buffer, sizeof(buffer), f)) {
            return -1;
        }
        
        trim_inplace(buffer);
        if (buffer[0] == '\0') {
            // Skip empty lines
            i--;
            continue;
        }
        
        // Parse space-separated values
        char *token = strtok(buffer, " \t");
        int j = 0;
        while (token != NULL && j < n) {
            adj[i * n + j] = (int) strtol(token, NULL, 10);
            token = strtok(NULL, " \t");
            j++;
        }
        
        if (j < n) {
            fprintf(stderr, "Warning: Row %d has only %d elements, expected %d\n", i + 1, j, n);
        }
    }
    return 0;
}

// --- Public Functions ---

void print_adj_matrix(const char *name, int n, const int *adj) {
    printf("Graph name='%s' n=%d\n", name, n);
    printf("    ");
    for (int c = 0; c < n; ++c) printf("%4d", c + 1);
    printf("\n");
    for (int r = 0; r < n; ++r) {
        printf("%4d", r + 1);
        for (int c = 0; c < n; ++c) {
            printf("%4d", adj[r * n + c]);
        }
        printf("\n");
    }
    printf("\n");
}

int load_graphs(const char *path, int *n_g, int **adj_g, int *n_h, int **adj_h) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", path);
        return 1;
    }

    char buffer[4096];
    
    // Read first graph (G)
    // Read number of vertices
    if (!fgets(buffer, sizeof(buffer), f)) {
        fprintf(stderr, "Failed to read number of vertices for graph G\n");
        fclose(f);
        return 2;
    }
    trim_inplace(buffer);
    *n_g = (int) strtol(buffer, NULL, 10);
    if (*n_g <= 0) {
        fprintf(stderr, "Invalid number of vertices for graph G: %d\n", *n_g);
        fclose(f);
        return 2;
    }
    
    // Allocate and read adjacency matrix for G
    *adj_g = calloc((size_t) (*n_g) * (size_t) (*n_g), sizeof(int));
    if (!*adj_g) {
        fprintf(stderr, "Memory allocation failed for graph G\n");
        fclose(f);
        return 3;
    }
    
    if (read_adjacency_matrix(f, *n_g, *adj_g) != 0) {
        fprintf(stderr, "Failed to read adjacency matrix for graph G\n");
        free(*adj_g);
        fclose(f);
        return 3;
    }
    
    // Read second graph (H)
    // Read number of vertices
    if (!fgets(buffer, sizeof(buffer), f)) {
        fprintf(stderr, "Failed to read number of vertices for graph H\n");
        free(*adj_g);
        fclose(f);
        return 4;
    }
    trim_inplace(buffer);
    *n_h = (int) strtol(buffer, NULL, 10);
    if (*n_h <= 0) {
        fprintf(stderr, "Invalid number of vertices for graph H: %d\n", *n_h);
        free(*adj_g);
        fclose(f);
        return 4;
    }
    
    // Allocate and read adjacency matrix for H
    *adj_h = calloc((size_t) (*n_h) * (size_t) (*n_h), sizeof(int));
    if (!*adj_h) {
        fprintf(stderr, "Memory allocation failed for graph H\n");
        free(*adj_g);
        fclose(f);
        return 5;
    }
    
    if (read_adjacency_matrix(f, *n_h, *adj_h) != 0) {
        fprintf(stderr, "Failed to read adjacency matrix for graph H\n");
        free(*adj_g);
        free(*adj_h);
        fclose(f);
        return 5;
    }
    
    fclose(f);
    return 0;
}
