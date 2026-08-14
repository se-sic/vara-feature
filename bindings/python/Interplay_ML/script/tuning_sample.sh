#!/bin/bash
#SBATCH --job-name=tuning
#SBATCH --output=/scratch/mani00001/slurm_logs/tuning_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=8G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL

set -e
cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)

for SYS in "${SYSTEMS[@]}"; do
    python -m bindings.python.Interplay_ML.Sampling.Controller tune "$SYS"
done