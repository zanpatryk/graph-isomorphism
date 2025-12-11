#!/bin/bash

# ==========================================
# Configuration
# ==========================================
EXECUTABLE="../aac"
DIR_EXACT="tests_exact"
DIR_APPROX="tests_approx"
RESULTS_DIR="results"
N_VALUE=1  # Number of solutions to find

# Check if binary exists
if [[ ! -f "$EXECUTABLE" ]]; then
    echo "Error: Executable '$EXECUTABLE' not found."
    exit 1
fi

# Create results directory (mkdir -p is safe if it exists)
mkdir -p "$RESULTS_DIR"

echo "=========================================="
echo "Running All Algorithm Tests"
echo "=========================================="
echo "Executable:      $EXECUTABLE"
echo "Exact Data:      $DIR_EXACT"
echo "Approx Data:     $DIR_APPROX"
echo "Results Dir:     $RESULTS_DIR"
echo "N Value:         $N_VALUE"
echo "=========================================="
echo ""

# ==========================================
# Function: Run a specific algorithm on a specific folder
# ==========================================
run_algo_suite() {
    local algo_name="$1"    # e.g., "iso_exact"
    local input_dir="$2"    # e.g., "tests_exact"

    # Skip if directory doesn't exist
    if [[ ! -d "$input_dir" ]]; then
        echo "Warning: Directory '$input_dir' not found. Skipping $algo_name."
        return
    fi

    local result_file="$RESULTS_DIR/${algo_name}_results.txt"

    # 1. Initialize Result File with Header
    # We use a block { ... } > file to write multiple lines efficiently
    {
        echo "=========================================="
        echo "Results for Algorithm: $algo_name"
        echo "=========================================="
        echo "Date: $(date)"
        echo "Source Directory: $input_dir"
        echo "N Value: $N_VALUE"
        echo "=========================================="
        echo ""
    } > "$result_file"

    echo "----------------------------------------"
    echo "Algorithm: $algo_name"
    echo "Reading from: $input_dir"
    echo "----------------------------------------"

    # 2. Iterate through files
    # shopt -s nullglob ensures the loop doesn't run if *.txt matches nothing
    shopt -s nullglob
    for file in "$input_dir"/*.txt; do
        local clean_name
        clean_name=$(basename "$file")

        # Write separator to the log file
        {
            echo ""
            echo "----------------------------------------"
            echo "Data File: $clean_name"
            echo "----------------------------------------"
        } >> "$result_file"

        # Run the command
        # Syntax: ./aac <algo> <file> <n>
        # 2>&1 captures errors into the same stream
        "$EXECUTABLE" "$algo_name" "$file" "$N_VALUE" >> "$result_file" 2>&1
        local run_status=$?

        # Console feedback
        if [[ $run_status -eq 0 ]]; then
            echo "  ✓ $clean_name"
        else
            echo "  ✗ $clean_name (FAILED)"
            echo "    [ERROR] Exit code $run_status" >> "$result_file"
        fi
    done
    shopt -u nullglob

    # Footer for the file
    {
        echo ""
        echo "=========================================="
        echo "End of results for: $algo_name"
        echo "=========================================="
    } >> "$result_file"

    echo "  -> Saved to $result_file"
    echo ""
}

# ==========================================
# Main Execution
# ==========================================

# --- PHASE 1: FINDING ALGORITHMS (Isomorphism) ---
echo ">>> PHASE 1: Subgraph Isomorphism (Finding) <<<"
echo ""

# 1. Exact Isomorphism (on Exact Data)
run_algo_suite "iso_exact" "$DIR_EXACT"

# 2. Approx Isomorphism (on Approx Data)
run_algo_suite "iso_approx" "$DIR_APPROX"


# --- PHASE 2: EXTENSION ALGORITHMS ---
echo ">>> PHASE 2: Minimal Extension <<<"
echo ""

# 3. Exact Extension (on Exact Data)
run_algo_suite "ext_exact" "$DIR_EXACT"

# 4. Approx Extension (on Approx Data)
run_algo_suite "ext_approx" "$DIR_APPROX"


# ==========================================
# Summary
# ==========================================
SUMMARY_FILE="$RESULTS_DIR/summary.txt"

{
    echo "=========================================="
    echo "Test Run Summary"
    echo "=========================================="
    echo "Date: $(date)"
    echo "Executable: $EXECUTABLE"
    echo ""
    echo "Generated Log Files:"

    shopt -s nullglob
    for f in "$RESULTS_DIR"/*.txt; do
        if [[ "$(basename "$f")" != "summary.txt" ]]; then
            echo "  - $f"
        fi
    done
    shopt -u nullglob

    echo "=========================================="
} > "$SUMMARY_FILE"

echo "=========================================="
echo "All tests completed!"
echo "Summary saved to: $SUMMARY_FILE"
echo "=========================================="