import random
import sys
import argparse

def generate_random_multigraph(size, density=0.3, max_multiedges=3):
    """
    Generates a directed multigraph adjacency matrix.
    """
    matrix = [[0] * size for _ in range(size)]
    for i in range(size):
        for j in range(size):
            if random.random() < density:
                matrix[i][j] = random.randint(1, max_multiedges)
    return matrix

def permute_matrix(matrix):
    """
    Scrambles the vertices of a graph.
    """
    n = len(matrix)
    p = list(range(n))
    random.shuffle(p)
    
    new_matrix = [[0] * n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            new_matrix[i][j] = matrix[p[i]][p[j]]
    return new_matrix

def extract_subgraph(H_matrix, sub_size):
    """
    Extracts a random subgraph G of size sub_size from H_matrix.
    """
    H_size = len(H_matrix)
    if sub_size > H_size:
        raise ValueError("Subgraph size cannot be larger than original graph.")

    indices = random.sample(range(H_size), sub_size)
    indices.sort()

    G_matrix = [[0] * sub_size for _ in range(sub_size)]
    for i in range(sub_size):
        for j in range(sub_size):
            G_matrix[i][j] = H_matrix[indices[i]][indices[j]]
            
    return G_matrix

def get_max_weight(matrix):
    """Returns the maximum value in the adjacency matrix."""
    return max(max(row) for row in matrix)

def main():
    # --- ARGUMENT PARSING ---
    # Adapts to the call signature from generate_tests.fish:
    # python3 graph_gen.py <g_size> <h_size> [--not-subgraph] --output <file>
    parser = argparse.ArgumentParser(description="Generate test graphs.")
    
    # Positional arguments (Order matters: $g_size then $h_size)
    parser.add_argument("g_size", type=int, help="Size of subgraph G")
    parser.add_argument("h_size", type=int, help="Size of main graph H")
    
    # Optional flags
    parser.add_argument("--not-subgraph", action="store_true", 
                        help="If set, G will NOT be a subgraph of H")
    parser.add_argument("--output", type=str, required=True, 
                        help="Output filename")

    args = parser.parse_args()

    # Map arguments to your logic variables
    SIZE_G = args.g_size
    SIZE_H = args.h_size
    IS_SUBGRAPH = not args.not_subgraph  # If flag is present, IS_SUBGRAPH is False
    OUTPUT_FILE = args.output
    # ---------------------

    # 1. Generate H
    H_base = generate_random_multigraph(SIZE_H, density=0.3, max_multiedges=4)

    # 2. Extract G
    G = extract_subgraph(H_base, SIZE_G)

    # 3. Apply Logic
    if IS_SUBGRAPH:
        # Case: G is a subgraph (Isomorphic)
        H_final = permute_matrix(H_base)
    else:
        # Case: G is NOT a subgraph (Non-Isomorphic)
        
        # FIX: Find the global max weight in H
        max_weight_in_H = get_max_weight(H_base)
        
        # Pick a random spot in G (preferably an existing edge to keep topology similar)
        edges = [(r, c) for r in range(SIZE_G) for c in range(SIZE_G) if G[r][c] > 0]
        
        if edges:
            r, c = random.choice(edges)
        else:
            r, c = random.randint(0, SIZE_G-1), random.randint(0, SIZE_G-1)
            
        # Set G's edge to be strictly heavier than anything in H
        # This guarantees G cannot be found in H
        G[r][c] = max_weight_in_H + 1
        
        # H is just scrambled
        H_final = permute_matrix(H_base)

    # 4. Write Output
    with open(OUTPUT_FILE, "w") as f:
        # Write G
        f.write(f"{SIZE_G}\n")
        for row in G:
            f.write(" ".join(map(str, row)) + "\n")
        
        # Write H
        f.write(f"{SIZE_H}\n")
        for row in H_final:
            f.write(" ".join(map(str, row)) + "\n")

    # (Optional) minimal output to avoid clogging the shell script logs
    # print(f"Saved to {OUTPUT_FILE}")

if __name__ == "__main__":
    main()