#!/bin/bash
#SBATCH --job-name=exp_array
#SBATCH --output=/scratch/miec00001/slurm_logs/exp_%A_%a.out
#SBATCH --time=08:00:00
#SBATCH --constraint=eku
#SBATCH --partition=anywhere
#SBATCH --mail-user=miec00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL

set -euo pipefail

ROOT="/scratch/miec00001/Thesis/vara-feature"
EXP_DIR="$ROOT/experiment"
RUN_DIR="$ROOT/experiment_runs/job_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}"

cd "$EXP_DIR"

LINE=$(sed -n "${SLURM_ARRAY_TASK_ID}p" experiment_params.txt)

if [ -z "$LINE" ]; then
  echo "No parameters for task ${SLURM_ARRAY_TASK_ID}"
  exit 1
fi

SYSTEM_PATH=$(echo "$LINE" | awk '{print $1}')
SYSTEM_NAME=$(echo "$LINE" | awk '{print $2}')
STRATEGY=$(echo "$LINE" | awk '{print $3}')
SOURCE_T=$(echo "$LINE" | awk '{print $4}')
ITERATION=$(echo "$LINE" | awk '{print $5}')

rm -rf "$RUN_DIR"
mkdir -p "$RUN_DIR"

cp experiment_config.py "$RUN_DIR/"
cp sample_wrapper.py "$RUN_DIR/"
cp experiment_evaluation1.py "$RUN_DIR/"
cp experiment_evaluation2.py "$RUN_DIR/"
cp experiment_evaluation3.py "$RUN_DIR/"
cp experiment_main.py "$RUN_DIR/"

cd "$RUN_DIR"

VARA_FEATURE_ROOT="$ROOT" \
PYTHONPATH="$ROOT/build/bindings/python/vara-feature:$ROOT/bindings/python:$EXP_DIR:$RUN_DIR" \
"$ROOT/.venv/bin/python" - <<PY
from pathlib import Path
import subprocess

from experiment_config import SamplingStrategy
from experiment_main import get_sample_size_from_twise
from sample_wrapper import generate_and_evaluate_sample

root = Path("/scratch/miec00001/Thesis/vara-feature")
system_path = (root / "$SYSTEM_PATH").resolve()
system_name = "$SYSTEM_NAME"
strategy = "$STRATEGY"
source_t = int("$SOURCE_T")
iteration = int("$ITERATION")
output_csv = Path("results.csv").resolve()

sample_size = get_sample_size_from_twise(system_path, source_t)

print(f"system_name={system_name}")
print(f"strategy={strategy}")
print(f"source_t={source_t}")
print(f"iteration={iteration}")
print(f"sample_size={sample_size}")
print(f"system_path={system_path}")
print(f"output_csv={output_csv}")

if strategy == SamplingStrategy.RANDOM.value:
    strategy_path = (root / "build/bin/experiment_preparation").resolve()
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