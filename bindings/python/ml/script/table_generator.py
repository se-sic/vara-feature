import os
import re
import sys
import argparse
from statistics import mean

# Define your configurations
measurement_feature_pairs = [
    ('sampling/measurements/7z.csv', 'sampling/feature_models/7z.xml'),
    ('sampling/measurements/lrzip.csv', 'sampling/feature_models/lrzip.xml'),
    ('sampling/measurements/Dune.csv', 'sampling/feature_models/Dune.xml'),
    ('sampling/measurements/BerkeleyDBC.csv', 'sampling/feature_models/BerkeleyDBC.xml'),
    ('sampling/measurements/Hipacc.csv', 'sampling/feature_models/Hipacc.xml'),
    ('sampling/measurements/LLVM.csv', 'sampling/feature_models/LLVM.xml'),
    ('sampling/measurements/Polly.csv', 'sampling/feature_models/Polly.xml'),
    ('sampling/measurements/x264.csv', 'sampling/feature_models/x264.xml'),
]

sample_sizes = [20, 50, 200]
sampling_strategies = [ 'distance', 'diversified-distance', 'random']

# Output directory base
base_output_dir = '../experiments_output_randomSeed_1'

# Log file name (adjust if different)
LOG_FILE_NAME = 'run.log'

# LaTeX table file
LATEX_TABLE_FILE = 'validation_errors_table_seed1.tex'

# Regular expression to extract Validation Error
error_pattern = re.compile(r'Validation Error:\s+([\d\.]+)')
PERCENT = "\\%"
NEW_LINE = "\\\\"


def extract_validation_errors(log_file_path):
    """
    Extracts all Validation Error values from the given log file.

    :param log_file_path: Path to the log file.
    :return: List of Validation Error floats.
    """
    errors = []
    if not os.path.isfile(log_file_path):
        print(f"Warning: Log file {log_file_path} does not exist.", file=sys.stderr)
        return errors

    with open(log_file_path, 'r') as file:
        for line in file:
            match = error_pattern.search(line)
            if match:
                try:
                    error = float(match.group(1))
                    errors.append(error)
                except ValueError:
                    print(f"Warning: Could not convert extracted error to float in line: {line.strip()}",
                          file=sys.stderr)
    return errors

def generate_latex_table(data, output_file):
    """Generates a complete LaTeX document with a table of mean Validation Errors.

    :param data: Nested dictionary containing mean errors for each configuration.
                 Format: {case_study: {sample_size: {strategy: mean_error}}}
    :param output_file: Path to the output LaTeX file.
    """
    with open(output_file, 'w') as tex_file:
        # Begin LaTeX document
        tex_file.write("\\documentclass{article}\n")
        tex_file.write("\\usepackage{booktabs}\n")
        tex_file.write("\\usepackage[margin=1in]{geometry}\n")  # Optional: Adjust margins
        tex_file.write("\\usepackage{caption}\n")  # Optional: For caption customization
        tex_file.write("\\begin{document}\n\n")

        tex_file.write("\\begin{table}[ht]\n")
        tex_file.write("\\centering\n")
        # Begin tabular environment with booktabs support
        tex_file.write("\\begin{tabular}{l" + " c" * len(sampling_strategies) + "}\n")
        tex_file.write("\\toprule\n")

        # Header
        header = "Case Study"
        for strategy in sampling_strategies:
            header += f" & {strategy.capitalize().replace('-', ' ')}"
        header += " \\\\\n"
        tex_file.write(header)
        tex_file.write("\\midrule\n")

        # Table Rows
        for mf_pair in measurement_feature_pairs:
            measurement = os.path.splitext(os.path.basename(mf_pair[0]))[0]
            feature = os.path.splitext(os.path.basename(mf_pair[1]))[0]
            case_study = f"{measurement}-{feature}"
            tex_file.write(f"\\textbf{{{case_study}}} & " + " & ".join([""] * len(sampling_strategies)) + " \\\\\n")
            for size in sample_sizes:
                row = f"$t={size}$"
                for strategy in sampling_strategies:
                    mean_error = data.get(case_study, {}).get(size, {}).get(strategy, None)
                    if mean_error is not None:
                        row += f" & {mean_error:.2f}{PERCENT}"
                    else:
                        row += " & --"
                row += " \\\\\n"
                tex_file.write(row)
        tex_file.write("\\bottomrule\n")
        tex_file.write("\\end{tabular}\n")

        # Optional: Add a caption and label
        tex_file.write("\\caption{Mean Validation Errors for Different Configurations}\n")
        tex_file.write("\\label{tab:validation_errors}\n")

        tex_file.write("\\end{table}\n\n")

        # End LaTeX document
        tex_file.write("\\end{document}\n")

def main():
    parser = argparse.ArgumentParser(description="Generate LaTeX table of mean Validation Errors from experiment logs.")
    parser.add_argument('--output', type=str, default=LATEX_TABLE_FILE, help='Output LaTeX file name.')
    args = parser.parse_args()

    # Data structure to hold mean Validation Errors
    # Format: {case_study: {sample_size: {strategy: mean_error}}}
    data = {}

    for measurement_csv, feature_xml in measurement_feature_pairs:
        measurement = os.path.splitext(os.path.basename(measurement_csv))[0]
        feature = os.path.splitext(os.path.basename(feature_xml))[0]
        case_study = f"{measurement}-{feature}"

        data[case_study] = {}

        for size in sample_sizes:
            data[case_study][size] = {}
            for strategy in sampling_strategies:
                # Construct the experiment directory name
                experiment_dir = f"{measurement}_{feature}_size{size}_{strategy}"
                experiment_path = os.path.join(base_output_dir, experiment_dir)

                log_file_path = os.path.join(experiment_path, LOG_FILE_NAME)

                errors = extract_validation_errors(log_file_path)

                if errors:
                    mean_error = mean(errors)
                    data[case_study][size][strategy] = mean_error
                else:
                    data[case_study][size][strategy] = None  # Indicate missing data

    # Generate LaTeX table
    generate_latex_table(data, args.output)
    print(f"LaTeX table generated at {args.output}")


if __name__ == "__main__":
    main()
