#!/bin/bash
#SBATCH --job-name=tsizes
#SBATCH --output=/scratch/mani00001/slurm_logs/tsizes_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=8G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-29

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
TS=(1 2 3)

IDX=$SLURM_ARRAY_TASK_ID
SYS=${SYSTEMS[$((IDX/3))]}
T=${TS[$((IDX%3))]}

srun bash /scratch/mani00001/vara-feature-1/bindings/python/Interplay_ML/script/run_tsizes.sh "$SYS" "$T"