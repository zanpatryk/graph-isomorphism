#!/bin/bash

# Configuration
C_PROG="./graph_gen"
DIR_EXACT="tests_exact"
DIR_APPROX="tests_approx"
NUM_CASES=10

# ---------------------------------------------------------
# Optional: Compile the C code if the executable is missing
# ---------------------------------------------------------
if [[ ! -f "$C_PROG" ]]; then
    if [[ -f "graph_gen.c" ]]; then
        echo "Executable '$C_PROG' not found. Compiling graph_gen.c..."
        gcc -O3 -o graph_gen graph_gen.c
        if [[ $? -ne 0 ]]; then
            echo "Error: Compilation failed."
            exit 1
        fi
    else
        echo "Error: Could not find executable '$C_PROG' or source 'graph_gen.c'"
        exit 1
    fi
fi

# Create output directories (mkdir -p creates only if missing)
mkdir -p "$DIR_EXACT"
mkdir -p "$DIR_APPROX"

echo "--- Starting Generation ---"

# ==========================================
# 1. Generate EXACT Algorithm Cases (Sizes 10-20)
# ==========================================
echo "[1/2] Generating $NUM_CASES cases for Exact Algorithms (Size 10-20)..."

for i in $(seq 1 $NUM_CASES); do
    # Generate random h_size between 10 and 20 (inclusive)
    # Bash $RANDOM is 0-32767.
    # Logic: $(( RANDOM % (MAX - MIN + 1) + MIN ))
    h_size=$(( (RANDOM % 11) + 10 ))

    max_g=$(( h_size - 1 ))

    # Generate random g_size between 4 and max_g
    range_g=$(( max_g - 4 + 1 ))
    g_size=$(( (RANDOM % range_g) + 4 ))

    # Determine Type (Alternating)
    if (( i % 2 == 0 )); then
        # Even: Non-Isomorphic
        flag="--not-subgraph"
        type_lbl="non_isomorphic"
    else
        # Odd: Isomorphic
        flag=""
        type_lbl="isomorphic"
    fi

    filename="$DIR_EXACT/exact_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"

    # Run the C executable
    # Note: We pass $flag unquoted so it expands to nothing if empty,
    # or passes the flag if present.
    $C_PROG $g_size $h_size $flag --output "$filename"
done

# ==========================================
# 2. Generate APPROX Algorithm Cases (Sizes 30-40)
# ==========================================
echo "[2/2] Generating $NUM_CASES cases for Approx Algorithms (Size 30-40)..."

for i in $(seq 1 $NUM_CASES); do
    # h_size between 30 and 40
    h_size=$(( (RANDOM % 11) + 30 ))

    max_g=$(( h_size - 5 ))

    # g_size between 10 and max_g
    range_g=$(( max_g - 10 + 1 ))
    g_size=$(( (RANDOM % range_g) + 10 ))

    # Determine Type (Alternating)
    if (( i % 2 == 0 )); then
        flag="--not-subgraph"
        type_lbl="non_isomorphic"
    else
        flag=""
        type_lbl="isomorphic"
    fi

    filename="$DIR_APPROX/heur_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"

    # Run
    $C_PROG $g_size $h_size $flag --output "$filename"
done

echo "--- Done! ---"
echo "Exact cases saved in: ./$DIR_EXACT"
echo "Approx cases saved in: ./$DIR_APPROX"