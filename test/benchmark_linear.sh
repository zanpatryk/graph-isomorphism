#!/bin/bash

# ============================================================================
# Benchmark Script: Linear Graph Size Scaling (n=5 to n=100)
# Generates test data for execution time comparison charts
# ============================================================================

# Configuration
C_SCRIPT="./graph_gen"
EXECUTABLE="../build/aac"
DIR_BENCHMARK="benchmark_linear"
RESULTS_DIR="results"
RESULTS_FILE="$RESULTS_DIR/benchmark_linear_results.csv"

# Size ranges
MIN_SIZE=5
MAX_SIZE=10
STEP_SIZE=5          # Generate graphs every 5 vertices
H_OFFSET=5           # H is always G + H_OFFSET vertices

# Limits for exact algorithms (they become too slow beyond this)
EXACT_ISO_LIMIT=25
EXACT_EXT_LIMIT=12

echo "=============================================="
echo "Benchmark: Linear Graph Size Scaling"
echo "=============================================="
echo "Graph sizes: G from $MIN_SIZE to $MAX_SIZE (step $STEP_SIZE)"
echo "H = G + $H_OFFSET"
echo "Exact algorithm limits: iso=$EXACT_ISO_LIMIT, ext=$EXACT_EXT_LIMIT"
echo "=============================================="

# ==========================================
# 0. Clean and Recreate Directories
# ==========================================
echo ""
echo "--- Preparing Directories ---"

if [ -d "$DIR_BENCHMARK" ]; then
    echo "Cleaning existing benchmark directory..."
    rm -rf "$DIR_BENCHMARK"
fi
mkdir -p "$DIR_BENCHMARK"
mkdir -p "$RESULTS_DIR"

# ==========================================
# 1. Generate Test Cases
# ==========================================
echo ""
echo "--- Generating Test Cases ---"

for (( g_size=MIN_SIZE; g_size<=MAX_SIZE; g_size+=STEP_SIZE )); do
    h_size=$((g_size + H_OFFSET))
    
    # Generate isomorphic case (G is subgraph of H)
    filename_iso="$DIR_BENCHMARK/bench_iso_G${g_size}_H${h_size}.txt"
    "$C_SCRIPT" "$g_size" "$h_size" --output "$filename_iso"
    
    # Generate non-isomorphic case (G is NOT subgraph of H) - for extension tests
    filename_ext="$DIR_BENCHMARK/bench_ext_G${g_size}_H${h_size}.txt"
    "$C_SCRIPT" "$g_size" "$h_size" --not-subgraph --output "$filename_ext"
    
    echo "Generated: G=$g_size, H=$h_size"
done

echo ""
echo "Generated $((((MAX_SIZE - MIN_SIZE) / STEP_SIZE + 1) * 2)) test files"

# ==========================================
# 2. Run Benchmarks and Collect Results
# ==========================================
echo ""
echo "--- Running Benchmarks ---"

# Function to extract time in ms from output (macOS compatible)
# Looks for pattern like "(370ms)" and extracts "370"
extract_time() {
    echo "$1" | sed -n 's/.*(\([0-9]*\)ms).*/\1/p' | tail -1
}

# Create CSV header
echo "g_size,h_size,iso_exact_ms,iso_approx_ms,ext_exact_ms,ext_approx_ms" > "$RESULTS_FILE"

for (( g_size=MIN_SIZE; g_size<=MAX_SIZE; g_size+=STEP_SIZE )); do
    h_size=$((g_size + H_OFFSET))
    
    filename_iso="$DIR_BENCHMARK/bench_iso_G${g_size}_H${h_size}.txt"
    filename_ext="$DIR_BENCHMARK/bench_ext_G${g_size}_H${h_size}.txt"
    
    echo -n "Testing G=$g_size, H=$h_size: "
    
    # Initialize times
    iso_exact_time="-"
    iso_approx_time="-"
    ext_exact_time="-"
    ext_approx_time="-"
    
    # Run iso_exact (only if within limit)
    if [ $g_size -le $EXACT_ISO_LIMIT ]; then
        echo -n "iso_exact "
        output=$("$EXECUTABLE" iso_exact "$filename_iso" 1 2>&1)
        iso_exact_time=$(extract_time "$output")
        if [ -z "$iso_exact_time" ]; then
            iso_exact_time="-"
        fi
    fi
    
    # Run iso_approx (always)
    echo -n "iso_approx "
    output=$("$EXECUTABLE" iso_approx "$filename_iso" 1 2>&1)
    iso_approx_time=$(extract_time "$output")
    if [ -z "$iso_approx_time" ]; then
        iso_approx_time="-"
    fi
    
    # Run ext_exact (only if within limit)
    if [ $g_size -le $EXACT_EXT_LIMIT ]; then
        echo -n "ext_exact "
        output=$("$EXECUTABLE" ext_exact "$filename_ext" 1 2>&1)
        ext_exact_time=$(extract_time "$output")
        if [ -z "$ext_exact_time" ]; then
            ext_exact_time="-"
        fi
    fi
    
    # Run ext_approx (always)
    echo -n "ext_approx "
    output=$("$EXECUTABLE" ext_approx "$filename_ext" 1 2>&1)
    ext_approx_time=$(extract_time "$output")
    if [ -z "$ext_approx_time" ]; then
        ext_approx_time="-"
    fi
    
    echo "done"
    
    # Write to CSV
    echo "$g_size,$h_size,$iso_exact_time,$iso_approx_time,$ext_exact_time,$ext_approx_time" >> "$RESULTS_FILE"
done

echo ""
echo "=============================================="
echo "Benchmark Complete!"
echo "=============================================="
echo "Results saved to: $RESULTS_FILE"
echo ""
echo "CSV Format:"
echo "  g_size, h_size, iso_exact_ms, iso_approx_ms, ext_exact_ms, ext_approx_ms"
echo ""
echo "Note: '-' indicates the algorithm was skipped (exceeded size limit)"
echo "      Exact iso limit: G <= $EXACT_ISO_LIMIT"
echo "      Exact ext limit: G <= $EXACT_EXT_LIMIT"
echo "=============================================="

# ==========================================
# 3. Generate Summary Statistics
# ==========================================
echo ""
echo "--- Quick Summary ---"
cat "$RESULTS_FILE"

