#!/bin/bash
#SBATCH --job-name=rq2_sampling
#SBATCH --output=/scratch/mani00001/slurm_logs/rq2_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=64G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-9

set -e
REPO=$(cd "$SLURM_SUBMIT_DIR/../../../.." && pwd)
cd "$REPO"
source .venv/bin/activate
export PYTHONPATH="$REPO/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
PROP=0.2
SYS=${SYSTEMS[$SLURM_ARRAY_TASK_ID]}

echo "Currently at $SYS"

python -m bindings.python.Interplay_ML.Sampling.Controller run 2 random "$SYS" --prop "$PROP"