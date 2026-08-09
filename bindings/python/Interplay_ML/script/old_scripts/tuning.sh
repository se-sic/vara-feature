#!/bin/bash
#SBATCH --job-name=tuning
#SBATCH --output=/scratch/mani00001/slurm_logs/tuning_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=64G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-9

cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
SYS=${SYSTEMS[$SLURM_ARRAY_TASK_ID]}

echo "Tuning $SLURM_ARRAY_TASK_ID: $SYS"
python -m bindings.python.Interplay_ML.ML.Tune system "$SYS"