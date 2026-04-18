import subprocess
import shutil
import pandas as pd
import experiment_evaluation1
import experiment_evaluation2
import experiment_evaluation3

from pathlib import Path
from experiment_config import SamplingStrategy, SamplingStrategies, systems
from sample_wrapper import generate_and_evaluate_sample


def get_sample_size_from_twise(system_path: Path, source_t: int):
    return 10
    # placeholder for later


def generate_random_sample_with_cpp(
    system_path: Path,
    sampling_strategy: SamplingStrategy,
    source_t: int,
    sample_size: int,
    iteration: int,
    output_csv: Path,
):
    strategy_path = Path("../build/bin/experiment_preparation").resolve()

    subprocess.run(
        [
            str(strategy_path),
            str(system_path),
            sampling_strategy.value,
            str(source_t),
            str(sample_size),
            str(iteration),
            str(output_csv),
        ],
        check=True,
    )


output_csv = Path("results.csv").resolve()

if output_csv.exists():
    output_csv.unlink()

for system_path, system_name in systems:
    for t_wise_sampling in [1, 2, 3]:
        sample_size = get_sample_size_from_twise(system_path, t_wise_sampling)

        for strategy in SamplingStrategies:
            for iteration in range(1, 2):
                if strategy == SamplingStrategy.RANDOM:
                    generate_random_sample_with_cpp(
                        system_path,
                        strategy,
                        t_wise_sampling,
                        sample_size,
                        iteration,
                        output_csv,
                    )
                else:
                    generate_and_evaluate_sample(
                        system_path=system_path,
                        system_name=system_name,
                        strategy=strategy.value,
                        source_t=t_wise_sampling,
                        sample_size=sample_size,
                        run=iteration,
                        output_csv=output_csv,
                    )

df = pd.read_csv(output_csv)
print(df.head())

experiment_evaluation1.main(output_csv)
experiment_evaluation2.main(output_csv)

evaluation3_output_csv = Path("evaluation3_result.csv").resolve()
shutil.copyfile(output_csv, evaluation3_output_csv)
experiment_evaluation3.main(evaluation3_output_csv)