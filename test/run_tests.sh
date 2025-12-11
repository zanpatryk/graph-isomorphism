#!/bin/bash

# ==========================================
# Configuration
# ==========================================
EXECUTABLE="../aac"
DIR_EXACT="tests_approx"
DIR_APPROX="tests_approx"
DIR_EXACT_EXT="tests_exact_ext"
DIR_APPROX_EXT="tests_approx_ext"
RESULTS_DIR="results"
N_VALUE=1  # Number of solutions to find

# Check if binary exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Executable '$EXECUTABLE' not found."
    exit 1
fi

# Create results directory
if [ ! -d "$RESULTS_DIR" ]; then
    mkdir -p "$RESULTS_DIR"
fi

echo "=========================================="
echo "Running All Algorithm Tests"
echo "=========================================="
echo "Executable:      $EXECUTABLE"
echo "Exact Data:      $DIR_EXACT"
echo "Approx Data:     $DIR_APPROX"
echo "Exact Ext Data:  $DIR_EXACT_EXT"
echo "Approx Ext Data: $DIR_APPROX_EXT"
echo "Results Dir:     $RESULTS_DIR"
echo "N Value:         $N_VALUE"
echo "Stop on Fail:    YES"
echo "=========================================="
echo ""

# ==========================================
# Function: Run a specific algorithm on a specific folder
# ==========================================
run_algo_suite() {
    local algo_name="$1"    # e.g., "iso_exact"
    local input_dir="$2"    # e.g., "tests_exact"

    # Skip if directory doesn't exist
    if [ ! -d "$input_dir" ]; then
        echo "Warning: Directory '$input_dir' not found. Skipping $algo_name."
        return
    fi

    local result_file="$RESULTS_DIR/${algo_name}_results.txt"

    # Enable nullglob so the loop doesn't run if no files exist
    shopt -s nullglob
    local data_files=("$input_dir"/*.txt)
    shopt -u nullglob

    # 1. Initialize Result File with Header
    echo "==========================================" > "$result_file"
    echo "Results for Algorithm: $algo_name" >> "$result_file"
    echo "==========================================" >> "$result_file"
    echo "Date: $(date)" >> "$result_file"
    echo "Source Directory: $input_dir" >> "$result_file"
    echo "N Value: $N_VALUE" >> "$result_file"
    echo "==========================================" >> "$result_file"
    echo "" >> "$result_file"

    echo "----------------------------------------"
    echo "Algorithm: $algo_name"
    echo "Reading from: $input_dir"
    echo "----------------------------------------"

    # 2. Iterate through files
    for file in "${data_files[@]}"; do
        local clean_name
        clean_name=$(basename "$file")

        # Write separator to the log file
        echo "" >> "$result_file"
        echo "----------------------------------------" >> "$result_file"
        echo "Data File: $clean_name" >> "$result_file"
        echo "----------------------------------------" >> "$result_file"

        # Capture start time (nanoseconds)
        # Note: %N is a GNU extension. If on macOS without gdate, this may need adjustment.
        local start_ts
        start_ts=$(date +%s%N)

        # Run the command
        # Syntax: ./aac <algo> <file> <n>
        "$EXECUTABLE" "$algo_name" "$file" "$N_VALUE" >> "$result_file" 2>&1

        # Capture status IMMEDIATELY
        local run_status=$?

        # Capture end time and calculate duration
        local end_ts
        end_ts=$(date +%s%N)

        # Calculate milliseconds (difference in nanoseconds / 1,000,000)
        local duration_ms=$(( (end_ts - start_ts) / 1000000 ))

        # Calculate seconds with 3 decimal places using awk (Bash math is integer only)
        local duration_sec
        duration_sec=$(awk -v ms="$duration_ms" 'BEGIN {printf "%.3f", ms/1000}')

        # Log time to file
        echo "" >> "$result_file"
        echo "[METADATA] Execution Time: ${duration_sec}s (${duration_ms}ms)" >> "$result_file"
        echo "[METADATA] Exit Code: $run_status" >> "$result_file"

        # Console feedback and Logic
        if [ "$run_status" -eq 0 ]; then
            echo "  ✓ $clean_name (${duration_ms}ms)"
        else
            echo "  ✗ $clean_name (FAILED)"
            echo "    [ERROR] Exit code $run_status. Check $result_file for details."
            echo ""
            echo "!!! ABORTING: Test failed. Stopping execution as requested. !!!"
            exit 1
        fi
    done

    # Footer for the file
    echo "" >> "$result_file"
    echo "==========================================" >> "$result_file"
    echo "End of results for: $algo_name" >> "$result_file"
    echo "==========================================" >> "$result_file"

    echo "  -> Saved to $result_file"
    echo ""
}

# ==========================================
# Main Execution
# ==========================================

# --- PHASE 1: FINDING ALGORITHMS (Isomorphism) ---
echo ">>> PHASE 1: Subgraph Isomorphism (Finding) <<<"
echo ""

# 1. Exact Isomorphism
run_algo_suite "iso_exact" "$DIR_EXACT"

# 2. Approx Isomorphism
run_algo_suite "iso_approx" "$DIR_APPROX"


# --- PHASE 2: EXTENSION ALGORITHMS ---
echo ">>> PHASE 2: Minimal Extension <<<"
echo ""

# 3. Exact Extension
run_algo_suite "ext_exact" "$DIR_EXACT_EXT"

# 4. Approx Extension
run_algo_suite "ext_approx" "$DIR_APPROX_EXT"


# ==========================================
# Summary
# ==========================================
SUMMARY_FILE="$RESULTS_DIR/summary.txt"
echo "==========================================" > "$SUMMARY_FILE"
echo "Test Run Summary" >> "$SUMMARY_FILE"
echo "==========================================" >> "$SUMMARY_FILE"
echo "Date: $(date)" >> "$SUMMARY_FILE"
echo "Status: SUCCESS (All tests passed)" >> "$SUMMARY_FILE"
echo "Executable: $EXECUTABLE" >> "$SUMMARY_FILE"
echo "" >> "$SUMMARY_FILE"
echo "Generated Log Files:" >> "$SUMMARY_FILE"

# Iterate over .txt files in results dir
for f in "$RESULTS_DIR"/*.txt; do
    if [ "$(basename "$f")" != "summary.txt" ]; then
        echo "  - $f" >> "$SUMMARY_FILE"
    fi
done

echo "==========================================" >> "$SUMMARY_FILE"

echo "=========================================="
echo "All tests completed successfully!"
echo "Summary saved to: $SUMMARY_FILE"
echo "=========================================="