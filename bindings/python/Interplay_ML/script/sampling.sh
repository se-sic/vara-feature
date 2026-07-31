#!/bin/bash
#SBATCH --job-name=rq1_sampling
#SBATCH --output=/scratch/mani00001/slurm_logs/rq1_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=64G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-29

set -e
cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
TS=(1 2 3)
#STRATEGY=(sbs dbs ddbs random rsbs)
IDX=$SLURM_ARRAY_TASK_ID
REM=$((IDX%30))
SYS=${SYSTEMS[$((REM/3))]}
T=${TS[$((REM%3))]}
#STRAT=${STRATEGY[$((IDX/30))]}

python -m bindings.python.Interplay_ML.Sampling.Controller run 1 random "$SYS" --t "$T"