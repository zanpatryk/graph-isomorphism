#!/bin/bash

# Script to run all algorithms on all data sets
# Results are organized by algorithm type in the results/ directory

# Don't exit on error - we want to continue even if some tests fail

# Configuration
EXECUTABLE="./build/aac"
DATA_DIR="./data"
RESULTS_DIR="./results"
N_VALUE=3  # Number of isomorphisms/extensions to find

# Algorithms to test
ALGORITHMS=("iso_exact" "iso_approx" "ext_exact" "ext_approx")

# Create results directory
mkdir -p "$RESULTS_DIR"

# Function to get all data files
get_data_files() {
    find "$DATA_DIR" -name "*.txt" -type f | sort
}

# Function to get a clean filename (without path and extension)
get_clean_name() {
    basename "$1" .txt
}

# Function to run a single test
run_test() {
    local algo=$1
    local data_file=$2
    local output_file=$3
    
    echo "Running $algo on $(basename $data_file)..."
    
    # Run the algorithm and capture output
    if "$EXECUTABLE" "$algo" "$data_file" "$N_VALUE" > "$output_file" 2>&1; then
        echo "  ✓ Success"
    else
        echo "  ✗ Failed (check $output_file)"
    fi
}

# Main execution
echo "=========================================="
echo "Running All Algorithm Tests"
echo "=========================================="
echo "Executable: $EXECUTABLE"
echo "Data directory: $DATA_DIR"
echo "Results directory: $RESULTS_DIR"
echo "N value: $N_VALUE"
echo "=========================================="
echo ""

# Get all data files
DATA_FILES=($(get_data_files))
TOTAL_TESTS=$((${#ALGORITHMS[@]} * ${#DATA_FILES[@]}))

echo "Found ${#DATA_FILES[@]} data files"
echo "Total tests to run: $TOTAL_TESTS"
echo ""

# Run tests for each algorithm type
for algo in "${ALGORITHMS[@]}"; do
    echo "----------------------------------------"
    echo "Algorithm: $algo"
    echo "----------------------------------------"
    
    # Create algorithm-specific result file
    ALGO_RESULT_FILE="$RESULTS_DIR/${algo}_results.txt"
    
    # Write header to algorithm result file
    {
        echo "=========================================="
        echo "Results for Algorithm: $algo"
        echo "=========================================="
        echo "Date: $(date)"
        echo "N value: $N_VALUE"
        echo "Total data files: ${#DATA_FILES[@]}"
        echo "=========================================="
        echo ""
    } > "$ALGO_RESULT_FILE"
    
    # Run tests for each data file
    for data_file in "${DATA_FILES[@]}"; do
        clean_name=$(get_clean_name "$data_file")
        
        # Write separator to algorithm result file
        {
            echo ""
            echo "----------------------------------------"
            echo "Data File: $clean_name ($(basename $data_file))"
            echo "----------------------------------------"
        } >> "$ALGO_RESULT_FILE"
        
        # Run the test and append output
        if "$EXECUTABLE" "$algo" "$data_file" "$N_VALUE" >> "$ALGO_RESULT_FILE" 2>&1; then
            echo "  ✓ $clean_name"
        else
            echo "  ✗ $clean_name (failed)"
            echo "    ERROR: Test failed for $clean_name" >> "$ALGO_RESULT_FILE"
        fi
        
        # Add separator after each test
        echo "" >> "$ALGO_RESULT_FILE"
    done
    
    # Write footer
    {
        echo ""
        echo "=========================================="
        echo "End of results for: $algo"
        echo "=========================================="
    } >> "$ALGO_RESULT_FILE"
    
    echo ""
done

# Create a summary file
SUMMARY_FILE="$RESULTS_DIR/summary.txt"
{
    echo "=========================================="
    echo "Test Run Summary"
    echo "=========================================="
    echo "Date: $(date)"
    echo "Executable: $EXECUTABLE"
    echo "Data directory: $DATA_DIR"
    echo "N value: $N_VALUE"
    echo ""
    echo "Algorithms tested:"
    for algo in "${ALGORITHMS[@]}"; do
        echo "  - $algo"
    done
    echo ""
    echo "Data files tested: ${#DATA_FILES[@]}"
    for data_file in "${DATA_FILES[@]}"; do
        echo "  - $(basename $data_file)"
    done
    echo ""
    echo "Results organized by algorithm:"
    for algo in "${ALGORITHMS[@]}"; do
        echo "  - $RESULTS_DIR/${algo}_results.txt"
    done
    echo ""
    echo "=========================================="
} > "$SUMMARY_FILE"

echo "=========================================="
echo "All tests completed!"
echo "=========================================="
echo "Results saved in: $RESULTS_DIR"
echo ""
echo "Summary: $SUMMARY_FILE"
echo ""
echo "Algorithm-specific results:"
for algo in "${ALGORITHMS[@]}"; do
    echo "  - $RESULTS_DIR/${algo}_results.txt"
done
echo ""

