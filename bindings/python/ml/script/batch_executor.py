import subprocess
import os
from concurrent.futures import ThreadPoolExecutor, as_completed
# Define your configurations
sample_sizes = [20, 50, 200]
sampling_strategies = ['solver', 'distance', 'diversified-distance','random']
measurement_feature_pairs = [
    ('../sampling/measurements/7z.csv', '../sampling/feature_models/7z.xml'),
    ('../sampling/measurements/lrzip.csv', '../sampling/feature_models/lrzip.xml'),
    ('../sampling/measurements/Dune.csv', '../sampling/feature_models/Dune.xml'),
    ('../sampling/measurements/BerkeleyDBC.csv', '../sampling/feature_models/BerkeleyDBC.xml'),
    ('../sampling/measurements/Hipacc.csv', '../sampling/feature_models/Hipacc.xml'),
    ('../sampling/measurements/LLVM.csv', '../sampling/feature_models/LLVM.xml'),
    ('../sampling/measurements/Polly.csv', '../sampling/feature_models/Polly.xml'),
    ('../sampling/measurements/x264.csv', '../sampling/feature_models/x264.xml'),
]

# Output directory base
base_output_dir = '../experiments_output'

def run_experiment(measurement_csv, feature_model_xml, sample_size, strategy):
    measurement_name = os.path.splitext(os.path.basename(measurement_csv))[0]
    feature_name = os.path.splitext(os.path.basename(feature_model_xml))[0]
    output_dir = os.path.join(
        base_output_dir,
        f"{measurement_name}_{feature_name}_size{sample_size}_{strategy}"
    )
    os.makedirs(output_dir, exist_ok=True)

    command = [
        'python', 'case_study.py',
        '--measurements_csv', measurement_csv,
        '--feature_model_xml', feature_model_xml,
        '--sample_seed', '42',
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
              f"SampleSize={sample_size}, Strategy={strategy}")
        subprocess.run(command, check=True)
        print(f"Completed: Output at {output_dir}\n")
    except subprocess.CalledProcessError as e:
        print(f"Failed: Measurement={measurement_csv}, FeatureModel={feature_model_xml}, "
              f"SampleSize={sample_size}, Strategy={strategy}")
        print(f"Error: {e}\n")

# Define the maximum number of workers (threads)
max_workers = 4  # Adjust based on your CPU cores and resource availability

with ThreadPoolExecutor(max_workers=max_workers) as executor:
    futures = []
    for measurement_csv, feature_model_xml in measurement_feature_pairs:
        for sample_size in sample_sizes:
            for strategy in sampling_strategies:
                futures.append(executor.submit(run_experiment, measurement_csv, feature_model_xml, sample_size, strategy))

    for future in as_completed(futures):
        future.result()  # To catch exceptions if any