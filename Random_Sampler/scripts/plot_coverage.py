#!/usr/bin/env python3
"""
Plot coverage curves from the coverage evaluation results.

Usage:
    python3 plot_coverage.py [directory]

Default directory: Random_Sampler/scripts/
"""

import sys
import os
import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path

def plot_coverage_file(filepath, output_path=None):
    """Plot coverage from a single CSV file."""
    # Read data
    df = pd.read_csv(filepath)
    
    # Get interaction size from filename
    filename = Path(filepath).name
    if '1wise' in filename:
        t_value = '1-wise'
    elif '2wise' in filename:
        t_value = '2-wise'
    elif '3wise' in filename:
        t_value = '3-wise'
    else:
        t_value = 'T-wise'
    
    # Create plot
    plt.figure(figsize=(12, 8))
    
    # Plot each metric
    metrics = [col for col in df.columns if col not in ['SampleSize', 'Percentage']]
    
    for metric in metrics:
        plt.plot(df['Percentage'], df[metric] * 100, 
                marker='o', linewidth=2, markersize=8, label=metric)
    
    plt.xlabel('Sample Size (%)', fontsize=12)
    plt.ylabel('Coverage (%)', fontsize=12)
    plt.title(f'{t_value} Feature Interaction Coverage - Random Sampling', fontsize=14, fontweight='bold')
    plt.legend(fontsize=10, loc='lower right')
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.xlim(0, 105)
    plt.ylim(0, 105)
    
    # Add value labels at the end
    for metric in metrics:
        final_value = df[metric].iloc[-1] * 100
        plt.annotate(f'{final_value:.1f}%', 
                    xy=(100, final_value),
                    xytext=(5, 0), textcoords='offset points',
                    fontsize=9, alpha=0.7)
    
    plt.tight_layout()
    
    # Save plot
    if output_path is None:
        output_path = filepath.replace('.csv', '.png')
    
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"✓ Saved plot: {output_path}")
    plt.close()

def plot_comparison(directory):
    """Plot comparison of different interaction sizes."""
    csv_files = list(Path(directory).glob('coverage_*wise.csv'))
    
    if len(csv_files) < 2:
        print("Not enough coverage files for comparison plot")
        return
    
    plt.figure(figsize=(14, 8))
    
    colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']
    
    for idx, filepath in enumerate(sorted(csv_files)):
        df = pd.read_csv(filepath)
        
        # Get t-value
        filename = filepath.name
        if '1wise' in filename:
            t_value = '1-wise'
        elif '2wise' in filename:
            t_value = '2-wise'
        elif '3wise' in filename:
            t_value = '3-wise'
        else:
            t_value = filename.replace('coverage_', '').replace('.csv', '')
        
        # Plot M6 (Default) metric only for comparison
        if 'M6_Default' in df.columns:
            plt.plot(df['Percentage'], df['M6_Default'] * 100,
                    marker='o', linewidth=2.5, markersize=8,
                    color=colors[idx % len(colors)],
                    label=f'{t_value} (M6 Default)')
    
    plt.xlabel('Sample Size (%)', fontsize=12)
    plt.ylabel('Coverage (%)', fontsize=12)
    plt.title('Coverage Comparison Across Interaction Sizes - Random Sampling', 
             fontsize=14, fontweight='bold')
    plt.legend(fontsize=11, loc='lower right')
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.xlim(0, 105)
    plt.ylim(0, 105)
    
    plt.tight_layout()
    
    output_path = os.path.join(directory, 'coverage_comparison.png')
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"✓ Saved comparison plot: {output_path}")
    plt.close()

def plot_metric_comparison(directory):
    """Plot comparison of different metrics for same interaction size."""
    # Use 2-wise as default, or first available
    csv_file = os.path.join(directory, 'coverage_2wise.csv')
    
    if not os.path.exists(csv_file):
        # Try to find any coverage file
        csv_files = list(Path(directory).glob('coverage_*wise.csv'))
        if not csv_files:
            print("No coverage files found for metric comparison")
            return
        csv_file = str(csv_files[0])
    
    df = pd.read_csv(csv_file)
    
    plt.figure(figsize=(14, 10))
    
    metrics = [col for col in df.columns if col not in ['SampleSize', 'Percentage']]
    colors = plt.cm.tab10(range(len(metrics)))
    
    for idx, metric in enumerate(metrics):
        plt.plot(df['Percentage'], df[metric] * 100,
                marker='o', linewidth=2, markersize=6,
                color=colors[idx], label=metric, alpha=0.8)
    
    plt.xlabel('Sample Size (%)', fontsize=12)
    plt.ylabel('Coverage (%)', fontsize=12)
    
    # Get t-value for title
    filename = Path(csv_file).name
    if '2wise' in filename:
        t_value = 'Pair-wise (t=2)'
    elif '1wise' in filename:
        t_value = 'One-wise (t=1)'
    elif '3wise' in filename:
        t_value = 'Three-wise (t=3)'
    else:
        t_value = 'T-wise'
    
    plt.title(f'{t_value} Coverage: Comparison of Metrics - Random Sampling',
             fontsize=14, fontweight='bold')
    plt.legend(fontsize=10, loc='lower right', ncol=2)
    plt.grid(True, alpha=0.3, linestyle='--')
    plt.xlim(0, 105)
    
    # Adjust y-axis to show differences better
    min_val = df[metrics].min().min() * 100
    max_val = df[metrics].max().max() * 100
    margin = (max_val - min_val) * 0.1
    plt.ylim(max(0, min_val - margin), min(105, max_val + margin))
    
    plt.tight_layout()
    
    output_path = os.path.join(directory, 'metrics_comparison.png')
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"✓ Saved metrics comparison: {output_path}")
    plt.close()

def main():
    # Get directory from command line or use default
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = 'Random_Sampler/scripts'
    
    if not os.path.exists(directory):
        print(f"Error: Directory not found: {directory}")
        return 1
    
    print(f"Plotting coverage results from: {directory}")
    
    # Find all coverage CSV files
    csv_files = list(Path(directory).glob('coverage_*wise.csv'))
    
    if not csv_files:
        print("No coverage CSV files found!")
        print("Expected files like: coverage_1wise.csv, coverage_2wise.csv")
        return 1
    
    print(f"Found {len(csv_files)} coverage file(s)")
    
    # Plot individual coverage files
    for filepath in csv_files:
        print(f"\nPlotting {filepath.name}...")
        plot_coverage_file(str(filepath))
    
    # Plot comparison across interaction sizes
    print("\nGenerating comparison plots...")
    plot_comparison(directory)
    
    # Plot metric comparison
    plot_metric_comparison(directory)
    
    print("\n✓ All plots generated successfully!")
    return 0

if __name__ == '__main__':
    sys.exit(main())
