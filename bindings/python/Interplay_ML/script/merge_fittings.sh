#!/bin/bash
#SBATCH --job-name=merge_fits
#SBATCH --output=/scratch/mani00001/slurm_logs/merge_fits_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=00:10:00
#SBATCH --mem=8G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL

set -e
REPO=$(cd "$SLURM_SUBMIT_DIR/../../../.." && pwd)
cd "$REPO"
source .venv/bin/activate
export PYTHONPATH="$REPO/build/bindings/python/vara-feature:$PYTHONPATH"

python -m bindings.python.Interplay_ML.ML.Learn merge