#!/usr/bin/env fish

# Configuration
set PYTHON_SCRIPT "graph_gen.py"
set DIR_EXACT "tests_exact"
set DIR_APPROX "tests_approx"
set NUM_CASES 10 

# Check if python script exists
if not test -f $PYTHON_SCRIPT
    echo "Error: Could not find '$PYTHON_SCRIPT'"
    exit 1
end

# Create output directories
if not test -d $DIR_EXACT
    mkdir $DIR_EXACT
end
if not test -d $DIR_APPROX
    mkdir $DIR_APPROX
end

echo "--- Starting Generation ---"

# ==========================================
# 1. Generate EXACT Algorithm Cases (Sizes 10-20)
# ==========================================
echo "[1/2] Generating $NUM_CASES cases for Exact Algorithms (Size 10-20)..."

for i in (seq 1 $NUM_CASES)
    set h_size (random 10 20)
    set max_g (math $h_size - 1)
    set g_size (random 4 $max_g)
    
    # 1. Start a fresh list of arguments for this iteration
    set cmd_args $g_size $h_size

    # 2. Determine Type (Alternating)
    if test (math $i % 2) -eq 0
        # Even: Non-Isomorphic
        set -a cmd_args "--not-subgraph"
        set type_lbl "non_isomorphic"
    else
        # Odd: Isomorphic
        # We do NOTHING to cmd_args here, so no flag is passed.
        set type_lbl "isomorphic"
    end

    set filename "$DIR_EXACT/exact_{$i}_{$type_lbl}_G{$g_size}_H{$h_size}.txt"
    
    # 3. Add output filename to args
    set -a cmd_args "--output" $filename

    # 4. Run Python with the clean list of args
    python3 $PYTHON_SCRIPT $cmd_args
end

# ==========================================
# 2. Generate APPROX Algorithm Cases (Sizes 30-40)
# ==========================================
echo "[2/2] Generating $NUM_CASES cases for Approx Algorithms (Size 30-40)..."

for i in (seq 1 $NUM_CASES)
    set h_size (random 30 40)
    set max_g (math $h_size - 5)
    set g_size (random 10 $max_g)

    # 1. Start fresh arguments list
    set cmd_args $g_size $h_size

    # 2. Determine Type (Alternating)
    if test (math $i % 2) -eq 0
        set -a cmd_args "--not-subgraph"
        set type_lbl "non_isomorphic"
    else
        set type_lbl "isomorphic"
    end

    set filename "$DIR_APPROX/heur_{$i}_{$type_lbl}_G{$g_size}_H{$h_size}.txt"
    
    # 3. Add output filename
    set -a cmd_args "--output" $filename
    
    # 4. Run
    python3 $PYTHON_SCRIPT $cmd_args
end

echo "--- Done! ---"
echo "Exact cases saved in: ./$DIR_EXACT"
echo "Approx cases saved in: ./$DIR_APPROX"