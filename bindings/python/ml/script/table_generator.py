import argparse
import os
import re
import sys
from statistics import mean

# Define regular expressions and configuration
completion_pattern = re.compile(r'Experimental pipeline completed successfully.')
export_pattern = re.compile(r'Trained model exported to (\S+)')
error_pattern = re.compile(r'Validation Error:\s+([\d\.]+)')

sampling_strategies = ['distance', 'diversified-distance', 'random', 'solver']
sample_sizes = [1, 2, 3]

LATEX_TABLE_FILE = 'validation_errors_table.tex'


def extract_log_data(log_file_path):
    """Extract relevant data from the log file."""
    if not os.path.isfile(log_file_path):
        print(f"Warning: Log file {log_file_path} does not exist.", file=sys.stderr)
        return None

    validation_error = None
    completed_successfully = False
    configuration_details = None

    with open(log_file_path, 'r') as file:
        for line in file:
            if completion_pattern.search(line):
                completed_successfully = True
            if export_match := export_pattern.search(line):
                configuration_details = export_match.group(1)
            if error_match := error_pattern.search(line):
                try:
                    validation_error = float(error_match.group(1))
                except ValueError:
                    print(f"Warning: Could not convert extracted error to float in line: {line.strip()}",
                          file=sys.stderr)
                    return None

    if not completed_successfully or validation_error is None or configuration_details is None:
        return None

    # Extract model, sample size, strategy, and seed from the directory path
    try:
        parts = configuration_details.split('/')
        config_dir = parts[-2]  # This assumes the format results/model_t=sample_strategy_seed
        model, t_part, strategy, seed_str = config_dir.split('_')
        sample_size = int(t_part.split('=')[1])
        seed = int(seed_str)
    except ValueError as e:
        print(f"Warning: Could not extract configuration details from {configuration_details}: {e}", file=sys.stderr)
        return None

    return model, sample_size, strategy, seed, validation_error


def collect_validation_errors(logs_directory):
    """Collect validation errors from all logs in the specified directory."""
    data = {}

    for log_filename in os.listdir(logs_directory):
        if log_filename.endswith('.log'):
            log_file_path = os.path.join(logs_directory, log_filename)
            result = extract_log_data(log_file_path)
            if result:
                model, sample_size, strategy, seed, validation_error = result
                if validation_error > 10000:
                    print(
                        f"Warning: Validation error {validation_error} seems too high for {log_filename}. Model: {model}, sample size: {sample_size}, strategy: {strategy}, seed: {seed}",
                        file=sys.stderr)
                    continue

                # Initialize data storage if necessary
                if model not in data:
                    data[model] = {}
                if strategy not in data[model]:
                    data[model][strategy] = {size: [] for size in sample_sizes}

                data[model][strategy][sample_size].append(validation_error)

    # Calculate means
    means = {}
    for model in data:
        means[model] = {}
        for strategy in data[model]:
            means[model][strategy] = {}
            for size in sample_sizes:
                if data[model][strategy][size]:
                    means[model][strategy][size] = mean(data[model][strategy][size])
                else:
                    means[model][strategy][size] = None

    return means


def generate_latex_table(data, output_file):
    """Generate a LaTeX table with the collected mean validation errors."""
    with open(output_file, 'w') as tex_file:
        tex_file.write("\\documentclass{article}\n")
        tex_file.write("\\usepackage{booktabs}\n")
        tex_file.write("\\usepackage{multirow}\n")
        tex_file.write("\\usepackage{adjustbox}\n")
        tex_file.write("\\begin{document}\n\n")
        tex_file.write("\\begin{table}[ht]\n")
        tex_file.write("\\begin{adjustbox}{max width=1.1\\textwidth,center}\n")

        # Define the number of columns (strategies * sample sizes + 1 for model names)
        num_columns = len(sampling_strategies) * len(sample_sizes) + 1
        tex_file.write("\\begin{tabular}{" + "l" + " c" * (num_columns - 1) + "}\n")
        tex_file.write("\\toprule\n")

        # Header row with strategy names and sample sizes
        header = "Model & " + " & ".join(
            f"\\multicolumn{{3}}{{c}}{{{strategy.capitalize()}}}" for strategy in sampling_strategies) + " \\\\\n"
        tex_file.write(header)

        # Sub-header with sample size
        sub_header = " & " + " & ".join(
            [f"$t={size}$" for _ in sampling_strategies for size in sample_sizes]) + " \\\\\n"
        tex_file.write("\\midrule\n")
        tex_file.write(sub_header)
        tex_file.write("\\midrule\n")

        # Table rows
        for model in data:
            row = f"\\textbf{{{model}}}"
            for strategy in sampling_strategies:
                for size in sample_sizes:
                    mean_error = data[model].get(strategy, {}).get(size, None)
                    if mean_error is not None:
                        row += f" & {mean_error:.2f}\\%"
                    else:
                        row += " & --"
            row += " \\\\\n"
            tex_file.write(row)

        tex_file.write("\\bottomrule\n")
        tex_file.write("\\end{tabular}\n")
        tex_file.write("\\end{adjustbox}\n")
        tex_file.write("\\caption{Mean Validation Errors for Different Configurations}\n")
        tex_file.write("\\label{tab:validation_errors}\n")
        tex_file.write("\\end{table}\n\n")
        tex_file.write("\\end{document}\n")


def main():
    parser = argparse.ArgumentParser(description="Generate LaTeX table of mean Validation Errors from experiment logs.")
    parser.add_argument('logs_directory', type=str, help='Directory containing log files.')
    parser.add_argument('--output', type=str, default=LATEX_TABLE_FILE, help='Output LaTeX file name.')
    args = parser.parse_args()

    # Collect validation errors
    data = collect_validation_errors(args.logs_directory)

    # Check if data was found
    if not data:
        print("No valid data found! Please check your log files.", file=sys.stderr)
        return

    # Generate LaTeX table
    generate_latex_table(data, args.output)
    print(f"LaTeX table generated at {args.output}")


if __name__ == "__main__":
    main()
