#!/bin/bash

# Configuration
C_SCRIPT="./graph_gen"
DIR_EXACT="tests_exact"
DIR_APPROX="tests_approx"
DIR_EXACT_EXT="tests_exact_ext"   # New directory for Exact Extension
DIR_APPROX_EXT="tests_approx_ext" # New directory for Approx Extension
NUM_CASES=10

# Check if python script exists
if [[ ! -f "$C_SCRIPT" ]]; then
    echo "Error: Could not find '$C_SCRIPT'"
    exit 1
fi

# Create output directories
[[ ! -d "$DIR_EXACT" ]] && mkdir -p "$DIR_EXACT"
[[ ! -d "$DIR_APPROX" ]] && mkdir -p "$DIR_APPROX"
[[ ! -d "$DIR_EXACT_EXT" ]] && mkdir -p "$DIR_EXACT_EXT"
[[ ! -d "$DIR_APPROX_EXT" ]] && mkdir -p "$DIR_APPROX_EXT"

echo "--- Starting Generation ---"

# ==========================================
# 1. Generate EXACT ISOMORPHISM Algorithm Cases (Sizes 10-20)
# ==========================================
echo "[1/4] Generating $NUM_CASES cases for Exact Isomorphism (Size 10-20)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Generate random sizes using shuf
    h_size=$(shuf -i 10-20 -n 1)
    max_g=$((h_size - 1))
    g_size=$(shuf -i 4-$max_g -n 1)

    # Start argument array
    cmd_args=( "$g_size" "$h_size" )

    # Determine Type (Alternating)
    if (( i % 2 == 0 )); then
        cmd_args+=( "--not-subgraph" )
        type_lbl="non_isomorphic"
    else
        type_lbl="isomorphic"
    fi

    filename="$DIR_EXACT/exact_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"
    cmd_args+=( "--output" "$filename" )
    "$C_SCRIPT" "${cmd_args[@]}"
done

# ==========================================
# 2. Generate APPROX ISOMORPHISM Algorithm Cases (Sizes 30-40)
# ==========================================
echo "[2/4] Generating $NUM_CASES cases for Approx Isomorphism (Size 30-40)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    h_size=$(shuf -i 30-40 -n 1)
    max_g=$((h_size - 5))
    g_size=$(shuf -i 10-$max_g -n 1)

    cmd_args=( "$g_size" "$h_size" )

    if (( i % 2 == 0 )); then
        cmd_args+=( "--not-subgraph" )
        type_lbl="non_isomorphic"
    else
        type_lbl="isomorphic"
    fi

    filename="$DIR_APPROX/heur_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"
    cmd_args+=( "--output" "$filename" )

    "$C_SCRIPT" "${cmd_args[@]}"
done

# ==========================================
# 3. Generate EXACT EXTENSION Cases (Max H=12, Non-Iso)
# ==========================================
echo "[3/4] Generating $NUM_CASES cases for Exact Extension (Max H=12)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Range 5 to 12
    h_size=$(shuf -i 5-12 -n 1)
    max_g=$((h_size - 1))
    g_size=$(shuf -i 3-$max_g -n 1)

    # Always Non-Isomorphic
    cmd_args=( "$g_size" "$h_size" "--not-subgraph" )
    type_lbl="non_isomorphic"

    filename="$DIR_EXACT_EXT/ext_exact_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"

    cmd_args+=( "--output" "$filename" )
    "$C_SCRIPT" "${cmd_args[@]}"
done

# ==========================================
# 4. Generate HEURISTIC EXTENSION Cases (Max H=20, Non-Iso)
# ==========================================
echo "[4/4] Generating $NUM_CASES cases for Heuristic Extension (Max H=20)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Range 10 to 20
    h_size=$(shuf -i 10-20 -n 1)
    max_g=$((h_size - 1))
    g_size=$(shuf -i 5-$max_g -n 1)

    # Always Non-Isomorphic
    cmd_args=( "$g_size" "$h_size" "--not-subgraph" )
    type_lbl="non_isomorphic"

    filename="$DIR_APPROX_EXT/ext_heur_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"

    cmd_args+=( "--output" "$filename" )
    "$C_SCRIPT" "${cmd_args[@]}"
done

echo "--- Done! ---"
echo "Exact cases saved in:      ./$DIR_EXACT"
echo "Approx cases saved in:     ./$DIR_APPROX"
echo "Exact Ext cases saved in:  ./$DIR_EXACT_EXT"
echo "Approx Ext cases saved in: ./$DIR_APPROX_EXT"