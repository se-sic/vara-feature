#!/bin/bash
#SBATCH --job-name=fitting_10k
#SBATCH --output=/scratch/mani00001/slurm_logs/fitting_10k_file_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=200G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-15

cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(VP9 JavaGC)
PROPS=(0.01 0.02 0.03 0.04 0.05 0.1 0.3 0.5)
IDX=$SLURM_ARRAY_TASK_ID
SYS=${SYSTEMS[$((IDX/8))]}
PROP=${PROPS[$((IDX%8))]}
OUT=/scratch/mani00001/vara-feature-1/bindings/python/Interplay_ML/Results_10k/rq_2_${SYS}_random_${PROP}.csv

echo "Currently at RQ2: $SYS with prop: $PROP"

python -u -c "from bindings.python.Interplay_ML.ML.Learn import run; run(2, '$SYS', 'random', '$OUT', only_prop=$PROP)"