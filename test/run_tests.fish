#!/usr/bin/env fish

# ==========================================
# Configuration
# ==========================================
set EXECUTABLE "../aac"
set DIR_EXACT "tests_exact"
set DIR_HEURISTIC "tests_heuristic"
set RESULTS_DIR "results"
set N_VALUE 1  # Number of solutions to find

# Check if binary exists
if not test -f $EXECUTABLE
    echo "Error: Executable '$EXECUTABLE' not found."
    exit 1
end

# Create results directory
if not test -d $RESULTS_DIR
    mkdir -p $RESULTS_DIR
end

echo "=========================================="
echo "Running All Algorithm Tests"
echo "=========================================="
echo "Executable:      $EXECUTABLE"
echo "Exact Data:      $DIR_EXACT"
echo "Heuristic Data:  $DIR_HEURISTIC"
echo "Results Dir:     $RESULTS_DIR"
echo "N Value:         $N_VALUE"
echo "=========================================="
echo ""

# ==========================================
# Function: Run a specific algorithm on a specific folder
# ==========================================
function run_algo_suite
    set algo_name $argv[1]    # e.g., "iso_exact"
    set input_dir $argv[2]    # e.g., "tests_exact"

    set result_file "$RESULTS_DIR/{$algo_name}_results.txt"
    set data_files $input_dir/*.txt
    
    # 1. Initialize Result File with Header
    echo "==========================================" > $result_file
    echo "Results for Algorithm: $algo_name" >> $result_file
    echo "==========================================" >> $result_file
    echo "Date: "(date) >> $result_file
    echo "Source Directory: $input_dir" >> $result_file
    echo "N Value: $N_VALUE" >> $result_file
    echo "==========================================" >> $result_file
    echo "" >> $result_file

    echo "----------------------------------------"
    echo "Algorithm: $algo_name"
    echo "Reading from: $input_dir"
    echo "----------------------------------------"

    # 2. Iterate through files
    for file in $data_files
        set clean_name (basename $file)
        
        # Write separator to the log file
        echo "" >> $result_file
        echo "----------------------------------------" >> $result_file
        echo "Data File: $clean_name" >> $result_file
        echo "----------------------------------------" >> $result_file

        # Run the command
        # Syntax: ./aac <algo> <file> <n>
        # 2>&1 captures errors into the same stream
        $EXECUTABLE $algo_name $file $N_VALUE >> $result_file 2>&1
        set run_status $status

        # Console feedback
        if test $run_status -eq 0
            echo "  ✓ $clean_name"
        else
            echo "  ✗ $clean_name (FAILED)"
            echo "    [ERROR] Exit code $run_status" >> $result_file
        end
    end
    
    # Footer for the file
    echo "" >> $result_file
    echo "==========================================" >> $result_file
    echo "End of results for: $algo_name" >> $result_file
    echo "==========================================" >> $result_file

    echo "  -> Saved to $result_file"
    echo ""
end

# ==========================================
# Main Execution
# ==========================================

# 1. Run EXACT algorithms on the Exact dataset
if test -d $DIR_EXACT
    run_algo_suite "iso_exact" $DIR_EXACT
    run_algo_suite "ext_exact" $DIR_EXACT
else
    echo "Warning: Directory '$DIR_EXACT' not found. Skipping Exact tests."
end

# 2. Run HEURISTIC algorithms on the Heuristic dataset
if test -d $DIR_HEURISTIC
    run_algo_suite "iso_approx" $DIR_HEURISTIC
    run_algo_suite "ext_approx" $DIR_HEURISTIC
else
    echo "Warning: Directory '$DIR_HEURISTIC' not found. Skipping Heuristic tests."
end

# ==========================================
# Summary
# ==========================================
set SUMMARY_FILE "$RESULTS_DIR/summary.txt"
echo "==========================================" > $SUMMARY_FILE
echo "Test Run Summary" >> $SUMMARY_FILE
echo "==========================================" >> $SUMMARY_FILE
echo "Date: "(date) >> $SUMMARY_FILE
echo "Executable: $EXECUTABLE" >> $SUMMARY_FILE
echo "" >> $SUMMARY_FILE
echo "Generated Log Files:" >> $SUMMARY_FILE
for f in $RESULTS_DIR/*.txt
    if test (basename $f) != "summary.txt"
        echo "  - $f" >> $SUMMARY_FILE
    end
end
echo "==========================================" >> $SUMMARY_FILE

echo "=========================================="
echo "All tests completed!"
echo "Summary saved to: $SUMMARY_FILE"
echo "=========================================="