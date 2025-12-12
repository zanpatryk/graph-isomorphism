#!/bin/bash

# --- CONFIGURATION ---
GENERATOR="./graph_gen"
SOLVER="../build/aac"
OUTPUT_DIR="./debug_data"
NUM_TRIALS=10

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

mkdir -p "$OUTPUT_DIR"

# Counters
total_tests=0
total_failures=0

run_batch() {
    local g_size=$1 # Main Graph
    local h_size=$2 # Subgraph

    echo "----------------------------------------------------"
    echo "Testing batch: Main Graph=$g_size, Subgraph=$h_size ($NUM_TRIALS trials)"

    local batch_failures=0

    for ((i=1; i<=NUM_TRIALS; i++)); do
        local test_name="G${g_size}_H${h_size}_id${i}"
        local test_file="${OUTPUT_DIR}/${test_name}.txt"

        # 1. Generate (Small First, then Big)
        $GENERATOR $h_size $g_size --output "$test_file" > /dev/null 2>&1
        if [ ! -f "$test_file" ]; then
            echo -e "  Trial $i: ${RED}GENERATION FAILED${NC}"
            exit 1
        fi

        # 2. Run Solver
        # Capturing stdout and stderr to variable
        output=$($SOLVER iso_approx "$test_file" 1 2>&1)

        # 3. Robust Check
        # We search for "Mapping" or "exists: YES" in the output
        if echo "$output" | grep -q "Mapping"; then
            echo -e "  Trial $i: ${GREEN}SUCCESS${NC}"
            rm "$test_file" # Delete success files
        else
            echo -e "  Trial $i: ${RED}FALSE NEGATIVE${NC}"
            ((batch_failures++))
            ((total_failures++))
            # Rename for debugging
            mv "$test_file" "${OUTPUT_DIR}/FAIL_${test_name}.txt"
        fi

        ((total_tests++))
    done

    # Batch summary
    if [ $batch_failures -gt 0 ]; then
        echo -e "Batch Result: ${RED}$batch_failures / $NUM_TRIALS failed${NC}"
    else
        echo -e "Batch Result: ${GREEN}100% Success${NC}"
    fi
}

# --- TEST PLAN ---

# Sanity Checks
run_batch 5 3
run_batch 10 4

# Medium Checks
run_batch 20 5
run_batch 50 10

# Large Checks
run_batch 100 15

# --- FINAL REPORT ---
echo "========================================"
if [ $total_tests -gt 0 ]; then
    rate=$(awk "BEGIN {print ($total_failures/$total_tests)*100}")
else
    rate="0"
fi
echo "Total Tests: $total_tests"
echo -e "False Negatives: ${RED}$total_failures${NC}"
echo -e "Failure Rate: ${RED}${rate}%${NC}"