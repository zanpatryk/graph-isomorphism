# Graph Extension Solver

## 1. Compile
Use `gcc` to compile the source code.
```bash
gcc main.c -o graph_solver
```

## 2. Prepare Input Data
Create a text file (e.g., `data.txt`) defining graph **g** and graph **h**.
* **Format:** Header (`g` or `h`), number of vertices, followed by edge list (`source-dest`).
* *Note: Vertices are 1-indexed.*

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

## 3. Execute
Run the program by specifying the function name and the input file path.

```bash
./graph_solver minimal_extension data.txt
```

### Output Example
```text
Minimal number of added edges: 1
1. Vertex Mapping (G -> H):
   G_1 -> H_1
   G_2 -> H_2
   G_3 -> H_3
2. Edges to Add to H:
   Add 1 edge(s): H_3 -> H_1  (for G_3 -> G_1)
```