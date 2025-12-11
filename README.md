# Graph Comparison Framework

A framework for comparing directed multigraphs, finding subgraph isomorphisms, and computing minimal extensions.

## Building

```bash
mkdir build && cd build
cmake ..
make
```

Or with gcc directly:
```bash
gcc -o aac main.c utils.c graph.c \
    algorithms/product_graph.c \
    algorithms/isomorphism.c \
    algorithms/minimal_extension.c \
    algorithms/minimal_extension_approximation.c
```

## Input Format

Create a text file defining graphs **G** and **H** using adjacency matrices:
- First line: number of vertices for graph G
- Next n lines: adjacency matrix for G (space-separated values, n×n matrix)
- Next line: number of vertices for graph H
- Next m lines: adjacency matrix for H (space-separated values, m×m matrix)

Each value in the adjacency matrix represents the number of edges from vertex i to vertex j (0-indexed internally, but displayed as 1-indexed).

```text
3
0 1 0
0 0 1
1 0 0
4
0 1 0 0
0 0 1 0
0 0 0 1
1 0 0 0
```

## Commands

### Finding Subgraph Isomorphisms

Find up to `n` distinct subgraph isomorphisms from G to H:

```bash
# Exact algorithm (Bron-Kerbosch on product graph)
./aac iso_exact data/graph.txt 3

# Heuristic algorithm (greedy clique)
./aac iso_approx data/graph.txt 3
```

### Finding Minimal Extensions

Find minimal edges to add to H so that G can be embedded `n` times:

```bash
# Exact algorithm (backtracking, iterative for n>1)
./aac ext_exact data/graph.txt 2

# Heuristic algorithm (greedy matching, iterative)
./aac ext_approx data/graph.txt 2
```

## Running All Tests

To run all algorithms on all data sets automatically, use the provided script:

```bash
# From the project root directory
./run_all_tests.sh
```

This script will:
- Run all 4 algorithms (`iso_exact`, `iso_approx`, `ext_exact`, `ext_approx`) on all data files
- Create a `results/` directory automatically
- Generate separate result files for each algorithm type:
  - `results/iso_exact_results.txt`
  - `results/iso_approx_results.txt`
  - `results/ext_exact_results.txt`
  - `results/ext_approx_results.txt`
- Create a `results/summary.txt` file with an overview of all tests


To regenerate all results, simply run:
```bash
./run_all_tests.sh
```

## Examples

```bash
# Find if triangle is subgraph of square (with diagonal needed)
./aac iso_exact data/triangle-square.txt 1

# Find 3 ways to embed triangle into itself
./aac iso_exact data/identical_triangles.txt 3

# Find minimal extension for 2 embeddings
./aac ext_approx data/compy.txt 2
```

## Output

### Isomorphism Output
```
--- Isomorphism Result ---
Subgraph isomorphism exists: YES
Number of isomorphisms found: 3

Mapping 1:
    G_1 -> H_1
    G_2 -> H_2
    G_3 -> H_3
...
```

### Extension Output
```
--- Minimal Extension Result ---
Total edges added: 2
Mappings found: 2

Mapping 1 (G -> H):
    G_1 -> H_1
    G_2 -> H_2

Mapping 2 (G -> H):
    G_1 -> H_3
    G_2 -> H_4

Extended H' adjacency matrix:
       1   2   3   4
   1   0   1   0   0
   2   0   0   1   1
   ...