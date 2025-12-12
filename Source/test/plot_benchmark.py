#!/usr/bin/env python3
"""
Benchmark Visualization Script
Generates execution time comparison charts from benchmark_linear_results.csv
"""

import csv
import matplotlib.pyplot as plt
import numpy as np
import os

# Configuration
RESULTS_FILE = "results/benchmark_linear_results.csv"
OUTPUT_DIR = "results/charts"

def load_data(filepath):
    """Load benchmark data from CSV file."""
    data = {
        'g_size': [],
        'h_size': [],
        'iso_exact': [],
        'iso_approx': [],
        'ext_exact': [],
        'ext_approx': []
    }
    
    with open(filepath, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            data['g_size'].append(int(row['g_size']))
            data['h_size'].append(int(row['h_size']))
            
            # Convert times, handling '-' as None
            for col in ['iso_exact_ms', 'iso_approx_ms', 'ext_exact_ms', 'ext_approx_ms']:
                key = col.replace('_ms', '')
                val = row[col]
                data[key].append(float(val) if val != '-' else None)
    
    return data

def plot_isomorphism_comparison(data, output_path):
    """Plot exact vs heuristic isomorphism algorithm comparison."""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    g_sizes = np.array(data['g_size'])
    iso_exact = np.array([v if v is not None else np.nan for v in data['iso_exact']])
    iso_approx = np.array([v if v is not None else np.nan for v in data['iso_approx']])
    
    # Plot data
    ax.plot(g_sizes[~np.isnan(iso_exact)], iso_exact[~np.isnan(iso_exact)], 
            'b-o', label='Exact Algorithm', linewidth=2, markersize=6)
    ax.plot(g_sizes, iso_approx, 
            'r-s', label='Heuristic Algorithm', linewidth=2, markersize=6)
    
    ax.set_xlabel('Graph G Size (vertices)', fontsize=12)
    ax.set_ylabel('Execution Time (ms)', fontsize=12)
    ax.set_title('Subgraph Isomorphism: Exact vs Heuristic Algorithm', fontsize=14)
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, max(g_sizes) + 5)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved: {output_path}")

def plot_extension_comparison(data, output_path):
    """Plot exact vs heuristic extension algorithm comparison."""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    g_sizes = np.array(data['g_size'])
    ext_exact = np.array([v if v is not None else np.nan for v in data['ext_exact']])
    ext_approx = np.array([v if v is not None else np.nan for v in data['ext_approx']])
    
    # Plot data
    ax.plot(g_sizes[~np.isnan(ext_exact)], ext_exact[~np.isnan(ext_exact)], 
            'b-o', label='Exact Algorithm', linewidth=2, markersize=6)
    ax.plot(g_sizes, ext_approx, 
            'r-s', label='Heuristic Algorithm', linewidth=2, markersize=6)
    
    ax.set_xlabel('Graph G Size (vertices)', fontsize=12)
    ax.set_ylabel('Execution Time (ms)', fontsize=12)
    ax.set_title('Minimal Extension: Exact vs Heuristic Algorithm', fontsize=14)
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, max(g_sizes) + 5)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved: {output_path}")

def plot_all_algorithms(data, output_path):
    """Plot all four algorithms on one chart."""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    g_sizes = np.array(data['g_size'])
    iso_exact = np.array([v if v is not None else np.nan for v in data['iso_exact']])
    iso_approx = np.array([v if v is not None else np.nan for v in data['iso_approx']])
    ext_exact = np.array([v if v is not None else np.nan for v in data['ext_exact']])
    ext_approx = np.array([v if v is not None else np.nan for v in data['ext_approx']])
    
    # Plot all algorithms
    ax.plot(g_sizes[~np.isnan(iso_exact)], iso_exact[~np.isnan(iso_exact)], 
            'b-o', label='Isomorphism (Exact)', linewidth=2, markersize=5)
    ax.plot(g_sizes, iso_approx, 
            'b--s', label='Isomorphism (Heuristic)', linewidth=2, markersize=5, alpha=0.7)
    ax.plot(g_sizes[~np.isnan(ext_exact)], ext_exact[~np.isnan(ext_exact)], 
            'g-o', label='Extension (Exact)', linewidth=2, markersize=5)
    ax.plot(g_sizes, ext_approx, 
            'g--s', label='Extension (Heuristic)', linewidth=2, markersize=5, alpha=0.7)
    
    ax.set_xlabel('Graph G Size (vertices)', fontsize=12)
    ax.set_ylabel('Execution Time (ms)', fontsize=12)
    ax.set_title('Algorithm Execution Time Comparison', fontsize=14)
    ax.legend(fontsize=10, loc='upper left')
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, max(g_sizes) + 5)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved: {output_path}")

def plot_log_scale(data, output_path):
    """Plot all algorithms with logarithmic y-axis."""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    g_sizes = np.array(data['g_size'])
    iso_exact = np.array([v if v is not None else np.nan for v in data['iso_exact']])
    iso_approx = np.array([v if v is not None else np.nan for v in data['iso_approx']])
    ext_exact = np.array([v if v is not None else np.nan for v in data['ext_exact']])
    ext_approx = np.array([v if v is not None else np.nan for v in data['ext_approx']])
    
    # Replace 0 with small value for log scale
    iso_exact = np.where(iso_exact == 0, 0.1, iso_exact)
    iso_approx = np.where(iso_approx == 0, 0.1, iso_approx)
    ext_exact = np.where(ext_exact == 0, 0.1, ext_exact)
    ext_approx = np.where(ext_approx == 0, 0.1, ext_approx)
    
    # Plot all algorithms
    ax.semilogy(g_sizes[~np.isnan(iso_exact)], iso_exact[~np.isnan(iso_exact)], 
                'b-o', label='Isomorphism (Exact)', linewidth=2, markersize=5)
    ax.semilogy(g_sizes, iso_approx, 
                'b--s', label='Isomorphism (Heuristic)', linewidth=2, markersize=5, alpha=0.7)
    ax.semilogy(g_sizes[~np.isnan(ext_exact)], ext_exact[~np.isnan(ext_exact)], 
                'g-o', label='Extension (Exact)', linewidth=2, markersize=5)
    ax.semilogy(g_sizes, ext_approx, 
                'g--s', label='Extension (Heuristic)', linewidth=2, markersize=5, alpha=0.7)
    
    ax.set_xlabel('Graph G Size (vertices)', fontsize=12)
    ax.set_ylabel('Execution Time (ms) - Log Scale', fontsize=12)
    ax.set_title('Algorithm Execution Time Comparison (Logarithmic Scale)', fontsize=14)
    ax.legend(fontsize=10, loc='upper left')
    ax.grid(True, alpha=0.3, which='both')
    ax.set_xlim(0, max(g_sizes) + 5)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved: {output_path}")

def plot_heuristics_only(data, output_path):
    """Plot only heuristic algorithms for full range."""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    g_sizes = np.array(data['g_size'])
    iso_approx = np.array([v if v is not None else np.nan for v in data['iso_approx']])
    ext_approx = np.array([v if v is not None else np.nan for v in data['ext_approx']])
    
    ax.plot(g_sizes, iso_approx, 
            'b-o', label='Isomorphism (Heuristic)', linewidth=2, markersize=6)
    ax.plot(g_sizes, ext_approx, 
            'g-s', label='Extension (Heuristic)', linewidth=2, markersize=6)
    
    ax.set_xlabel('Graph G Size (vertices)', fontsize=12)
    ax.set_ylabel('Execution Time (ms)', fontsize=12)
    ax.set_title('Heuristic Algorithms: Execution Time Scaling (n=5 to 100)', fontsize=14)
    ax.legend(fontsize=11)
    ax.grid(True, alpha=0.3)
    ax.set_xlim(0, max(g_sizes) + 5)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    plt.close()
    print(f"Saved: {output_path}")

def generate_latex_table(data, output_path):
    """Generate LaTeX table for the report."""
    with open(output_path, 'w') as f:
        f.write("% Auto-generated benchmark results table\n")
        f.write("\\begin{table}[h]\n")
        f.write("\\centering\n")
        f.write("\\small\n")
        f.write("\\begin{tabular}{|c|c|c|c|c|c|}\n")
        f.write("\\hline\n")
        f.write("$|V(G)|$ & $|V(H)|$ & Iso Exact & Iso Heur & Ext Exact & Ext Heur \\\\\n")
        f.write("\\hline\n")
        
        for i in range(len(data['g_size'])):
            g = data['g_size'][i]
            h = data['h_size'][i]
            ie = f"{int(data['iso_exact'][i])}" if data['iso_exact'][i] is not None else "--"
            ia = f"{int(data['iso_approx'][i])}" if data['iso_approx'][i] is not None else "--"
            ee = f"{int(data['ext_exact'][i])}" if data['ext_exact'][i] is not None else "--"
            ea = f"{int(data['ext_approx'][i])}" if data['ext_approx'][i] is not None else "--"
            f.write(f"{g} & {h} & {ie} & {ia} & {ee} & {ea} \\\\\n")
        
        f.write("\\hline\n")
        f.write("\\end{tabular}\n")
        f.write("\\caption{Execution time (ms) for linear graph size scaling}\n")
        f.write("\\label{tab:benchmark_linear}\n")
        f.write("\\end{table}\n")
    
    print(f"Saved: {output_path}")

def main():
    # Check if results file exists
    if not os.path.exists(RESULTS_FILE):
        print(f"Error: Results file not found: {RESULTS_FILE}")
        print("Please run benchmark_linear.sh first.")
        return 1
    
    # Create output directory
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    
    # Load data
    print(f"Loading data from: {RESULTS_FILE}")
    data = load_data(RESULTS_FILE)
    print(f"Loaded {len(data['g_size'])} data points")
    
    # Generate plots
    print("\nGenerating charts...")
    plot_isomorphism_comparison(data, f"{OUTPUT_DIR}/isomorphism_comparison.png")
    plot_extension_comparison(data, f"{OUTPUT_DIR}/extension_comparison.png")
    plot_all_algorithms(data, f"{OUTPUT_DIR}/all_algorithms.png")
    plot_log_scale(data, f"{OUTPUT_DIR}/all_algorithms_log.png")
    plot_heuristics_only(data, f"{OUTPUT_DIR}/heuristics_scaling.png")
    
    # Generate LaTeX table
    print("\nGenerating LaTeX table...")
    generate_latex_table(data, f"{OUTPUT_DIR}/benchmark_table.tex")
    
    print("\nDone! Charts saved to:", OUTPUT_DIR)
    return 0

if __name__ == "__main__":
    exit(main())

