#ifndef AAC_GRAPH_H
#define AAC_GRAPH_H

void print_adj_matrix(const char *name, int n, const int *adj);

int load_graphs(const char *path, int *n_g, int **adj_g, int *n_h, int **adj_h);

#endif //AAC_GRAPH_H