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
MIN_SIZE=3
MAX_SIZE=150
STEP_SIZE=1          
H_OFFSET=3           # H is always G + H_OFFSET vertices

# Limits for exact algorithms (they become too slow beyond this)
EXACT_ISO_LIMIT=30
EXACT_EXT_LIMIT=9

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

# Function to measure execution time in milliseconds (macOS compatible)
# Uses perl for sub-second timing since macOS date doesn't support %N
measure_time_ms() {
    local cmd="$1"
    local file="$2"
    local n="$3"
    
    # Get start time in milliseconds using perl
    local start_ms=$(perl -MTime::HiRes=time -e 'printf "%.0f", time * 1000')
    
    # Run command (discard output)
    "$EXECUTABLE" "$cmd" "$file" "$n" > /dev/null 2>&1
    
    # Get end time in milliseconds
    local end_ms=$(perl -MTime::HiRes=time -e 'printf "%.0f", time * 1000')
    
    # Calculate duration
    echo $((end_ms - start_ms))
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
        iso_exact_time=$(measure_time_ms "iso_exact" "$filename_iso" 1)
    fi
    
    # Run iso_approx (always)
    echo -n "iso_approx "
    iso_approx_time=$(measure_time_ms "iso_approx" "$filename_iso" 1)
    
    # Run ext_exact (only if within limit)
    if [ $g_size -le $EXACT_EXT_LIMIT ]; then
        echo -n "ext_exact "
        ext_exact_time=$(measure_time_ms "ext_exact" "$filename_ext" 1)
    fi
    
    # Run ext_approx (always)
    echo -n "ext_approx "
    ext_approx_time=$(measure_time_ms "ext_approx" "$filename_ext" 1)
    
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

