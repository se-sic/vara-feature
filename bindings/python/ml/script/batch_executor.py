import os
import argparse
sample_sizes = {
    '7z': [39, 600, 4091],
    'BerkeleyDBC': [15, 97, 343],
    'Dune': [25, 265, 1071],
    'Hipacc': [50, 843, 4601],
    'JavaGC': [32, 468, 3504],
    'LLVM': [11, 55, 165],
    'Polly': [28, 345, 2172],
    'VP9': [31, 483, 3893],
    'lrzip': [18, 90, 178],
    'x264': [12, 65, 212]
}

measurement_feature_pairs = [
    ('ml/sampling/measurements/7z.csv', 'ml/sampling/feature_models/7z.xml'),
    ('ml/sampling/measurements/lrzip.csv', 'ml/sampling/feature_models/lrzip.xml'),
    ('ml/sampling/measurements/Dune.csv', 'ml/sampling/feature_models/Dune.xml'),
    ('ml/sampling/measurements/BerkeleyDBC.csv', 'ml/sampling/feature_models/BerkeleyDBC.xml'),
    ('ml/sampling/measurements/Hipacc.csv', 'ml/sampling/feature_models/Hipacc.xml'),
    ('ml/sampling/measurements/LLVM.csv', 'ml/sampling/feature_models/LLVM.xml'),
    ('ml/sampling/measurements/Polly.csv', 'ml/sampling/feature_models/Polly.xml'),
    ('ml/sampling/measurements/x264.csv', 'ml/sampling/feature_models/x264.xml'),
    ('ml/sampling/measurements/JavaGC.csv', 'ml/sampling/feature_models/JavaGC.xml'),
    ('ml/sampling/measurements/VP9.csv', 'ml/sampling/feature_models/VP9.xml')
]

sampling_strategies = ['distance', 'diversified-distance', 'random']

parser = argparse.ArgumentParser(description='Run experiment based on Slurm job array.')
parser.add_argument('--index', type=int, required=True, help='Job array index from Slurm')

args = parser.parse_args()

total_measurements = len(measurement_feature_pairs)
n_seeds = 100
n_sampling_strategies = len(sampling_strategies)
sample_size = 3

# Calculate indices for the first segment: Distance, Diversified-Distance, and Random
job_id = args.index - 1

if job_id < total_measurements * n_seeds * n_sampling_strategies * sample_size:
    measurement_index = job_id // (n_seeds * n_sampling_strategies * sample_size)
    remaining_index = job_id % (n_seeds * n_sampling_strategies * sample_size)

    sample_size_index = remaining_index // (n_seeds * n_sampling_strategies)
    strategy_and_seed_index = remaining_index % (n_seeds * n_sampling_strategies)

    strategy_index = strategy_and_seed_index // n_seeds
    seed_index = strategy_and_seed_index % n_seeds

    selected_strategy = sampling_strategies[strategy_index]  # (0, 1, 2) -> (distance, diversified-distance, random)
    seed = seed_index + 1

# Calculate indices for the second segment: Solver
else:
    adjusted_index = job_id - total_measurements * n_seeds * n_sampling_strategies * sample_size
    measurement_index = adjusted_index // sample_size
    sample_size_index = adjusted_index % sample_size

    selected_strategy = 'solver'
    seed = 1

# Retrieve files and sample size
measurement_csv, feature_model_xml = measurement_feature_pairs[measurement_index]
sample_size = sample_sizes[os.path.splitext(os.path.basename(measurement_csv))[0]][sample_size_index]

# Construct the output directory
output_dir = f"results/{os.path.splitext(os.path.basename(measurement_csv))[0]}_t={sample_size_index + 1}_{selected_strategy}_{seed}"
os.makedirs(output_dir, exist_ok=True)

# Execute the command
command = (
    f"python case_study.py --measurements_csv {measurement_csv} "
    f"--feature_model_xml {feature_model_xml} --sample_seed {seed} "
    f"--sample_strategy {selected_strategy} --sample_size {sample_size} "
    f"--max_interaction_order 3 --margin 0.01 --threshold 0.01 "
    f"--learning_seed 42 --output_dir {output_dir}"
)
os.system(command)