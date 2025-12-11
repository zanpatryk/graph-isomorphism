# Graph Comparison Framework

A framework for comparing directed multigraphs, finding subgraph isomorphisms, and computing minimal extensions.

## Cloning & Building

```bash
git clone https://github.com/zanpatryk/graph-isomorphism
cd graph-isomorphism
mkdir build && cd build
cmake ..
make
```

Or with gcc directly:
```bash
gcc -o aac ../main.c ../utils.c ../graph.c \
    ../algorithms/product_graph.c \
    ../algorithms/isomorphism.c \
    ../algorithms/minimal_extension.c \
    ../algorithms/minimal_extension_approximation.c
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
./aac iso_exact ../data/graph.txt 3

# Heuristic algorithm (greedy clique)
./aac iso_approx ../data/graph.txt 3
```

### Finding Minimal Extensions

Find minimal edges to add to H so that G can be embedded `n` times:

```bash
# Exact algorithm (backtracking, iterative for n>1)
./aac ext_exact ../data/graph.txt 2

# Heuristic algorithm (greedy matching, iterative)
./aac ext_approx ../data/graph.txt 2
```

### Generating Random Graphs for Testing

In order to generate random graphs for testing purposes, there is a script in the test directory `generate_tests.sh`.
When executed, the script will generate 10 random graphs for each of the 4 algorithms (iso_exact, iso_approx, ext_exact, ext_approx)
with the parameters defined within the file (lines 11-34). Then use `run_tests.sh` to run the tests on the newly generated graph.

**Important**: `graph_gen` is already pre-compiled for x86_64 systems, you might need to re-compile the `graph_gen.c` code using
```bash
gcc graph_gen.c -o graph_gen
```

```bash
cd test

# Generate random graphs (for each algorithm, 10 graphs)
./generate_tests.sh

# Run all the tests (make sure you have built the project in ../build)
./run_tests.sh

# By providing 4 numbers to the run_tests, you can specify the N value for each algorithm:
# ISO_EXACT - Find 1 subgraph of graph G in graph H
# ISO_APPROX - Find 4 different subgraphs of graph G in graph H using approximation
# EXT_EXACT - Find 2 different extensions of graph H H1 and H2 such that G is a subgraph of H1 and H2
# EXT_APPROX - Find 6 different extensions of graph H H1, H2, ..., H6
# such that G is a subgraph of H1, H2, ..., H6 using approximation
./run_tests.sh 1 4 2 6
```