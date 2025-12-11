#!/bin/bash

# Configuration
C_SCRIPT="./graph_gen"
DIR_EXACT="tests_exact"
DIR_APPROX="tests_approx"
DIR_EXACT_EXT="tests_exact_ext"   # New directory for Exact Extension
DIR_APPROX_EXT="tests_approx_ext" # New directory for Approx Extension
NUM_CASES=10

ISO_EXACT_G_LOW=4
ISO_EXACT_G_MAX_DIFF=1
ISO_EXACT_H_LOW=10
ISO_EXACT_H_MAX=20

ISO_APPROX_G_LOW=10
ISO_APPROX_G_MAX_DIFF=1
ISO_APPROX_H_LOW=30
ISO_APPROX_H_MAX=40

EXT_EXACT_G_LOW=5
EXT_EXACT_G_MAX_DIFF=2
EXT_EXACT_H_LOW=7
EXT_EXACT_H_MAX=11

EXT_APPROX_G_LOW=50
EXT_APPROX_G_MAX_DIFF=1
EXT_APPROX_H_LOW=100
EXT_APPROX_H_MAX=200


echo "--- Preparing Directories ---"

# ==========================================
# 0. Clean and Recreate Directories
# ==========================================
# We define an array of the directories for easier handling
DIRS=("$DIR_EXACT" "$DIR_APPROX" "$DIR_EXACT_EXT" "$DIR_APPROX_EXT")

for d in "${DIRS[@]}"; do
    if [ -d "$d" ]; then
        echo "Cleaning existing directory: $d"
        rm -rf "$d"
    fi
    mkdir -p "$d"
done

echo "--- Starting Generation ---"

# ==========================================
# 1. Generate EXACT ISOMORPHISM Algorithm Cases
# ==========================================
echo "[1/4] Generating $NUM_CASES cases for Exact Isomorphism (Size $ISO_EXACT_H_LOW-$ISO_EXACT_H_MAX)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Generate random sizes using shuf
    h_size=$(shuf -i $ISO_EXACT_H_LOW-$ISO_EXACT_H_MAX -n 1)
    max_g=$((h_size - $ISO_EXACT_G_MAX_DIFF))
    g_size=$(shuf -i $ISO_EXACT_G_LOW-$max_g -n 1)

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
# 2. Generate APPROX ISOMORPHISM Algorithm Cases
# ==========================================
echo "[2/4] Generating $NUM_CASES cases for Approx Isomorphism (Size $ISO_APPROX_H_LOW-$ISO_APPROX_H_MAX)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    h_size=$(shuf -i $ISO_APPROX_H_LOW-$ISO_APPROX_H_MAX -n 1)
    max_g=$((h_size - $ISO_APPROX_G_MAX_DIFF))
    g_size=$(shuf -i $ISO_APPROX_G_LOW-$max_g -n 1)

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
# 3. Generate EXACT EXTENSION Cases
# ==========================================
echo "[3/4] Generating $NUM_CASES cases for Exact Extension (Max H=$EXT_EXACT_H_MAX)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Range 5 to 12
    h_size=$(shuf -i $EXT_EXACT_H_LOW-$EXT_EXACT_H_MAX -n 1)
    max_g=$((h_size - $EXT_EXACT_G_MAX_DIFF))
    g_size=$(shuf -i $EXT_EXACT_G_LOW-$max_g -n 1)

    # Always Non-Isomorphic
    cmd_args=( "$g_size" "$h_size" "--not-subgraph" )
    type_lbl="non_isomorphic"

    filename="$DIR_EXACT_EXT/ext_exact_${i}_${type_lbl}_G${g_size}_H${h_size}.txt"

    cmd_args+=( "--output" "$filename" )
    "$C_SCRIPT" "${cmd_args[@]}"
done

# ==========================================
# 4. Generate HEURISTIC EXTENSION Cases
# ==========================================
echo "[4/4] Generating $NUM_CASES cases for Heuristic Extension (Max H=$EXT_APPROX_H_MAX)..."

for (( i=1; i<=NUM_CASES; i++ )); do
    # Range 10 to 20
    h_size=$(shuf -i $EXT_APPROX_H_LOW-$EXT_APPROX_H_MAX -n 1)
    max_g=$((h_size - $EXT_APPROX_G_MAX_DIFF))
    g_size=$(shuf -i $EXT_APPROX_G_LOW-$max_g -n 1)

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