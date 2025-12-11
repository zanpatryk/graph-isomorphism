import random
import sys
import argparse

def generate_random_multigraph(size, density=0.3, max_multiedges=3):
    """
    Generates a directed multigraph adjacency matrix.
    size: Number of vertices
    density: Probability of an edge existing
    max_multiedges: Maximum weight (multiplicity) of an edge
    """
    matrix = [[0] * size for _ in range(size)]
    for i in range(size):
        for j in range(size):
            if random.random() < density:
                matrix[i][j] = random.randint(1, max_multiedges)
    return matrix

def permute_matrix(matrix):
    """
    Scrambles the vertices of a graph (Isomorphism).
    Returns the new matrix.
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
    indices = random.sample(range(H_size), sub_size)
    indices.sort()

    G_matrix = [[0] * sub_size for _ in range(sub_size)]
    for i in range(sub_size):
        for j in range(sub_size):
            G_matrix[i][j] = H_matrix[indices[i]][indices[j]]
            
    return G_matrix

def main():
    # 1. Setup Argument Parser
    parser = argparse.ArgumentParser(description="Generate Subgraph Isomorphism Test Cases")

    # Positional arguments (Required)
    parser.add_argument("size_g", type=int, help="Size of the smaller pattern graph G")
    parser.add_argument("size_h", type=int, help="Size of the larger target graph H")

    # Optional flags
    parser.add_argument("-o", "--output", type=str, help="Output filename (default: {size_g}_{size_h}_graph.txt)")
    parser.add_argument("--not-subgraph", action="store_true", help="Generate a negative case (G is NOT a subgraph of H)")
    
    # Configuration options
    parser.add_argument("--density", type=float, default=0.25, help="Graph edge density (0.0 to 1.0)")
    parser.add_argument("--max-weight", type=int, default=4, help="Maximum edge multiplicity/weight")

    args = parser.parse_args()

    # 2. Validation
    if args.size_g > args.size_h:
        print(f"Error: Size G ({args.size_g}) cannot be larger than Size H ({args.size_h}).")
        sys.exit(1)

    # 3. Determine Filename
    if args.output:
        output_file = args.output
    else:
        output_file = f"{args.size_g}_{args.size_h}_graph.txt"

    # 4. Logic Execution
    # Determine boolean state
    is_subgraph = not args.not_subgraph 

    # Generate Base H
    H_base = generate_random_multigraph(args.size_h, density=args.density, max_multiedges=args.max_weight)

    # Extract G
    G = extract_subgraph(H_base, args.size_g)

    if is_subgraph:
        H_final = permute_matrix(H_base)
        print(f"Generating CASE: G is a subgraph of H.")
    else:
        # Mutate G to ensure it is likely NOT a subgraph
        # We add weight to a random edge in G, making it 'heavier' than the extracted part of H
        r, c = random.randint(0, args.size_g-1), random.randint(0, args.size_g-1)
        G[r][c] += 1 
        
        H_final = permute_matrix(H_base)
        print(f"Generating CASE: G is NOT a subgraph (Minimal Extension required).")

    # 5. Write to File
    try:
        with open(output_file, "w") as f:
            f.write(f"{args.size_g}\n")
            for row in G:
                f.write(" ".join(map(str, row)) + "\n")
            
            f.write(f"{args.size_h}\n")
            for row in H_final:
                f.write(" ".join(map(str, row)) + "\n")

        print(f"Successfully created '{output_file}' with G={args.size_g}, H={args.size_h}")
        
    except IOError as e:
        print(f"Error writing to file: {e}")

if __name__ == "__main__":
    main()