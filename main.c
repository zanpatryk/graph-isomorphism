#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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

//Extract the first integer found in token and return it converted to 0-based index
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

static void print_adj_matrix(const char *name, int n, const int *adj) {
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
    const char *name = lines[header_idx];
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
        if (!lhs || !rhs) {
            fprintf(stderr, "Skipping malformed edge line: '%s'\n", lines[i]);
            free(copy);
            continue;
        }
        trim_inplace(lhs); trim_inplace(rhs);
        int u = parse_vertex_index(lhs);
        int v = parse_vertex_index(rhs);
        if (u < 0 || v < 0 || u >= n || v >= n) {
            fprintf(stderr, "Skipping edge with invalid vertex '%s' (n=%d)\n", lines[i], n);
            free(copy);
            continue;
        }
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
        fprintf(stderr, "Failed to read file: %s\n", path);
        return 1;
    }

    ssize_t idx_g = -1, idx_h = -1;
    find_header_indices(lines, nlines, &idx_g, &idx_h);

    if(idx_g < 0 || idx_h < 0){
        fprintf(stderr, "Both headers 'g' and 'h' must be present in %s\n", path);
        fprintf(stderr, "Lines read (showing with indexes):\n");
        for (size_t j = 0; j < nlines; ++j) {
            fprintf(stderr, "%3zu: (len=%zu) '%s'\n", j, strlen(lines[j]), lines[j]);
        }
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 2;
    }

    if (parse_graph_at(lines, nlines, (size_t)idx_g, n_g, adj_g) != 0) {
        fprintf(stderr, "Failed parsing graph 'g'\n");
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 3;
    }
    if (parse_graph_at(lines, nlines, (size_t)idx_h, n_h, adj_h) != 0) {
        fprintf(stderr, "Failed parsing graph 'h'\n");
        free(*adj_g);
        for (size_t j = 0; j < nlines; ++j) free(lines[j]);
        free(lines);
        return 4;
    }

    for (size_t j = 0; j < nlines; ++j) free(lines[j]);
    free(lines);
    return 0;
}

int main(void) {
    int *adj_g = NULL, *adj_h = NULL;
    int n_g = 0, n_h = 0;
    const char *path = "data/graphs.txt";

    if (load_graphs(path, &n_g, &adj_g, &n_h, &adj_h) != 0) return 1;

    print_adj_matrix("g", n_g, adj_g);
    print_adj_matrix("h", n_h, adj_h);

    free(adj_g);
    free(adj_h);
    return 0;
}
