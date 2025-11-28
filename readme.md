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

Create a text file defining graphs **G** and **H**:
- Header line: `g` or `h`
- Second line: number of vertices
- Remaining lines: edges as `source-dest` (1-indexed)

```text
g
3
1-2
2-3
3-1

h
4
1-2
2-3
3-4
4-1
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

## Examples

```bash
# Find if triangle is subgraph of square (with diagonal needed)
./aac iso_exact ../data/triangle-square.txt 1

# Find 3 ways to embed triangle into itself
./aac iso_exact ../data/identical_triangles.txt 3

# Find minimal extension for 2 embeddings
./aac ext_approx ../data/compy.txt 2
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