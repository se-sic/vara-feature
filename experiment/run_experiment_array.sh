#!/bin/bash
#SBATCH --job-name=exp_array
#SBATCH --output=/scratch/miec00001/slurm_logs/exp_%A_%a.out
#SBATCH --time=03:00:00
#SBATCH --constraint=eku
#SBATCH --partition=anywhere
#SBATCH --mail-user=miec00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL

set -e
cd /scratch/miec00001/Thesis/vara-feature/experiment || exit 1

LINE=$(sed -n "${SLURM_ARRAY_TASK_ID}p" experiment_params.txt)

if [ -z "$LINE" ]; then
  echo "No parameters for task ${SLURM_ARRAY_TASK_ID}"
  exit 1
fi

SYSTEM_PATH=$(echo "$LINE" | awk '{print $1}')
SYSTEM_NAME=$(echo "$LINE" | awk '{print $2}')
STRATEGY=$(echo "$LINE" | awk '{print $3}')
SOURCE_T=$(echo "$LINE" | awk '{print $4}')
SAMPLE_SIZE=$(echo "$LINE" | awk '{print $5}')
ITERATION=$(echo "$LINE" | awk '{print $6}')

RUN_DIR="/scratch/miec00001/Thesis/vara-feature/experiment_runs/job_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}"
rm -rf "$RUN_DIR"
mkdir -p "$RUN_DIR"

cp experiment_config.py "$RUN_DIR/"
cp experiment_evaluation1.py "$RUN_DIR/"
cp experiment_evaluation2.py "$RUN_DIR/"
cp experiment_evaluation3.py "$RUN_DIR/"

cd "$RUN_DIR"

VARA_FEATURE_ROOT="/scratch/miec00001/Thesis/vara-feature" \
PYTHONPATH="/scratch/miec00001/Thesis/vara-feature/build/bindings/python/vara-feature:/scratch/miec00001/Thesis/vara-feature/bindings/python:/scratch/miec00001/Thesis/vara-feature/experiment" \
/scratch/miec00001/Thesis/vara-feature/.venv/bin/python - <<PY
from pathlib import Path
from experiment_config import SamplingStrategy
from sample_wrapper import generate_and_evaluate_sample
import subprocess

system_path = (Path("/scratch/miec00001/Thesis/vara-feature") / "$SYSTEM_PATH").resolve()
system_name = "$SYSTEM_NAME"
strategy = "$STRATEGY"
source_t = int("$SOURCE_T")
sample_size = int("$SAMPLE_SIZE")
iteration = int("$ITERATION")
output_csv = Path("results.csv").resolve()

if strategy == SamplingStrategy.RANDOM.value:
    strategy_path = Path("/scratch/miec00001/Thesis/vara-feature/build/bin/experiment_preparation").resolve()
    subprocess.run(
        [
            str(strategy_path),
            str(system_path),
            strategy,
            str(source_t),
            str(sample_size),
            str(iteration),
            str(output_csv),
        ],
        check=True,
    )
else:
    generate_and_evaluate_sample(
        system_path=system_path,
        system_name=system_name,
        strategy=strategy,
        source_t=source_t,
        sample_size=sample_size,
        run=iteration,
        output_csv=output_csv,
    )

print("done")
PY
