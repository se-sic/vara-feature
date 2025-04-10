#!/bin/bash
#SBATCH --job-name=distance-based-validation
#SBATCH --output=/scratch/qido00001/logs/log_%A_%a.log
#SBATCH --array=1-9030%10  # Updated range for the new logic
#SBATCH --constraint=kine|(eku&i5)
#SBATCH --cpus-per-task=1
#SBATCH --mem=2G
#SBATCH --time=01:00:00

srun ./run.sh $SLURM_ARRAY_TASK_ID