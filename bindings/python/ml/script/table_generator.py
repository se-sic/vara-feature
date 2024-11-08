import os
import re
import sys
import argparse
from statistics import mean

# Define the sampling strategies
sampling_strategies = [
    'distance',
    'diversified-distance',
    'random',
    'solver'
]

# Define measurement-feature pairs
measurement_feature_pairs = [
    ('../sampling/measurements/7z.csv', '../sampling/feature_models/7z.xml'),
    ('../sampling/measurements/lrzip.csv', '../sampling/feature_models/lrzip.xml'),
    ('../sampling/measurements/Dune.csv', '../sampling/feature_models/Dune.xml'),
    ('../sampling/measurements/BerkeleyDBC.csv', '../sampling/feature_models/BerkeleyDBC.xml'),
    ('../sampling/measurements/Hipacc.csv', '../sampling/feature_models/Hipacc.xml'),
    ('../sampling/measurements/LLVM.csv', '../sampling/feature_models/LLVM.xml'),
    ('../sampling/measurements/Polly.csv', '../sampling/feature_models/Polly.xml'),
    ('../sampling/measurements/x264.csv', '../sampling/feature_models/x264.xml'),
    ('../sampling/measurements/JavaGC.csv', '../sampling/feature_models/JavaGC.xml'),
    ('../sampling/measurements/VP9.csv', '../sampling/feature_models/VP9.xml')
]

# Define sample sizes (t values)
sample_sizes = [1, 2, 3]  # Adjust this list as needed

# Base output directory
base_output_dir = '../experiments_output_randomSeed_1'

# Log file name (assuming it's consistent across all runs)
LOG_FILE_NAME = 'run.log'

# LaTeX table output file
LATEX_TABLE_FILE = 'validation_errors_table_seed1.tex'

# Regular expression pattern to extract Validation Error
error_pattern = re.compile(r'Validation Error:\s+([\d\.]+)')

# LaTeX formatting constants
PERCENT = "\\%"
NEW_LINE = "\\\\"


def extract_validation_error(log_file_path):
    """
    Extracts Validation Error value from the given log file.

    :param log_file_path: Path to the log file.
    :return: Validation Error float.
    """
    if not os.path.isfile(log_file_path):
        print(f"Warning: Log file {log_file_path} does not exist.", file=sys.stderr)
        return
    with open(log_file_path, 'r') as file:
        for line in file:
            match = error_pattern.search(line)
            if match:
                try:
                    error = float(match.group(1))
                    break
                except ValueError:
                    print(f"Warning: Could not convert extracted error to float in line: {line.strip()}",
                          file=sys.stderr)
    return error


def generate_latex_table(data, output_file):
    """
    Generates a complete LaTeX document with a table of mean Validation Errors.

    :param data: Nested dictionary containing mean errors for each configuration.
                 Format: {measurement: {strategy: {t_key: mean_error}}}
    :param output_file: Path to the output LaTeX file.
    """
    with open(output_file, 'w') as tex_file:
        # Begin LaTeX document
        tex_file.write("\\documentclass{article}\n")
        tex_file.write("\\usepackage{booktabs}\n")
        tex_file.write("\\usepackage[margin=1in]{geometry}\n")
        tex_file.write("\\usepackage{caption}\n")
        tex_file.write("\\begin{document}\n\n")

        tex_file.write("\\begin{table}[ht]\n")
        tex_file.write("\\centering\n")

        # Define number of strategies for table columns
        tex_file.write("\\begin{tabular}{l" + " c" * len(sampling_strategies) + "}\n")
        tex_file.write("\\toprule\n")

        # Header
        header = "Case Study & " + " & ".join(
            [strategy.capitalize().replace('-', ' ') for strategy in sampling_strategies]) + " \\\\\n"
        tex_file.write(header)
        tex_file.write("\\midrule\n")

        # Table Rows
        for measurement_csv, feature_xml in measurement_feature_pairs:
            measurement = os.path.splitext(os.path.basename(measurement_csv))[0]
            # For simplicity, using only measurement as case study (ignoring feature)
            case_study = f"{measurement}"
            tex_file.write(f"\\textbf{{{case_study}}} & " + " & ".join([""] * len(sampling_strategies)) + " \\\\\n")
            for size in sample_sizes:
                t_key = f"t={size}"
                row = f"$t={size}$"
                for strategy in sampling_strategies:
                    mean_error = data.get(measurement, {}).get(strategy, {}).get(t_key, None)
                    if mean_error is not None:
                        row += f" & {mean_error:.2f}{PERCENT}"
                    else:
                        row += " & --"
                row += " \\\\\n"
                tex_file.write(row)
        tex_file.write("\\bottomrule\n")
        tex_file.write("\\end{tabular}\n")
        tex_file.write("\\caption{Mean Validation Errors for Different Configurations}\n")
        tex_file.write("\\label{tab:validation_errors}\n")
        tex_file.write("\\end{table}\n\n")

        tex_file.write("\\end{document}\n")


def main():
    parser = argparse.ArgumentParser(description="Generate LaTeX table of mean Validation Errors from experiment logs.")
    parser.add_argument('--output', type=str, default=LATEX_TABLE_FILE, help='Output LaTeX file name.')
    args = parser.parse_args()

    # Data structure to hold mean Validation Errors
    # Format: {measurement: {strategy: {t_key: mean_error}}}
    data = {}

    for measurement_csv, feature_xml in measurement_feature_pairs:
        measurement = os.path.splitext(os.path.basename(measurement_csv))[0]
        # Initialize measurement in data dictionary
        if measurement not in data:
            data[measurement] = {}
        for size in sample_sizes:
            t_key = f"t={size}"
            for strategy in sampling_strategies:
                # Initialize strategy in measurement dictionary
                if strategy not in data[measurement]:
                    data[measurement][strategy] = {}

                # Determine seed range based on strategy
                if strategy == 'solver':
                    seeds = [42]  # Only one run for 'solver' strategy
                else:
                    seeds = range(1, 101)  # Seeds 1 to 100 for other strategies

                errors = []
                for seed in seeds:
                    # Construct the experiment directory name
                    output_dir = os.path.join(base_output_dir, f"{measurement}_t={size}_{strategy}_seed{seed}")
                    log_file_path = os.path.join(output_dir, LOG_FILE_NAME)

                    # Extract validation errors from the log file
                    run_error = extract_validation_error(log_file_path)

                    if run_error:

                        errors.append(run_error)

                # Calculate mean error if any errors were found
                if errors:
                    mean_error = mean(errors)
                    data[measurement][strategy][t_key] = mean_error
                else:
                    data[measurement][strategy][t_key] = None  # Indicate missing data

    # Generate LaTeX table with the collected data
    generate_latex_table(data, args.output)
    print(f"LaTeX table generated at {args.output}")


if __name__ == "__main__":
    main()