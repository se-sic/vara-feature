import subprocess
import os
from concurrent.futures import ThreadPoolExecutor, as_completed

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
    'X264': [12, 65, 212]
}


sampling_strategies = [ 'distance', 'diversified-distance', 'random', 'solver']

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

# Output directory base
base_output_dir = '../first_run/'


def run_experiment(measurement_csv, feature_model_xml, sample_size, t, strategy, sample_seed):
    measurement_name = os.path.splitext(os.path.basename(measurement_csv))[0]

    output_dir = os.path.join(
        base_output_dir,
        f"{measurement_name}_t={t}_{strategy}_seed{sample_seed}"
    )
    os.makedirs(output_dir, exist_ok=True)
    command = [
        'python', 'case_study.py',
        '--measurements_csv', measurement_csv,
        '--feature_model_xml', feature_model_xml,
        '--sample_seed', str(sample_seed),
        '--sample_strategy', strategy,
        '--sample_size', str(sample_size),
        '--max_interaction_order', '3',
        '--margin', '0.01',
        '--threshold', '0.01',
        '--learning_seed', '42',
        '--output_dir', output_dir
    ]
    try:
        print(f"Starting: Measurement={measurement_csv}, FeatureModel={feature_model_xml}, "
              f"SampleSize={sample_size}, Strategy={strategy}, SampleSeed={sample_seed}")
        subprocess.run(command, check=True)
        print(f"Completed: Output at {output_dir}\n")
    except subprocess.CalledProcessError as e:
        print(f"Failed: Measurement={measurement_csv}, FeatureModel={feature_model_xml}, "
              f"SampleSize={sample_size}, Strategy={strategy}, SampleSeed={sample_seed}")
        print(f"Error: {e}\n")


# Define the maximum number of workers (threads)
max_workers = 4  # Adjust based on your CPU cores and resource availability

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    futures = []
    for measurement_csv, feature_model_xml in measurement_feature_pairs:
        # Determine which sample sizes to use based on the file name
        file_key = os.path.splitext(os.path.basename(measurement_csv))[0]
        for i,sample_size in  enumerate(sample_sizes.get(file_key, []),start = 1):
            strategy = 'solver' # Solver strategy is deterministic
            futures.append(
                executor.submit(run_experiment, measurement_csv, feature_model_xml, sample_size, i, strategy, 42))
            for strategy in sampling_strategies:
                for seed in range(1, 100):  # Run seeds from 1 to 100
                    futures.append(
                        executor.submit(run_experiment, measurement_csv, feature_model_xml, sample_size, i, strategy,
                                        seed))

    for future in as_completed(futures):
        future.result()  # To catch exceptions if any