#!/bin/bash
#SBATCH --job-name=fitting
#SBATCH --output=/scratch/mani00001/slurm_logs/fitting_file_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=64G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-69

cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
RQ1_STRATS=(random twise sbs rsbs dbs ddbs)
IDX=$SLURM_ARRAY_TASK_ID
RESULTS=/scratch/mani00001/vara-feature-1/bindings/python/Interplay_ML/Results_10k

if [ $IDX -lt 60 ]; then 
    STRAT=${RQ1_STRATS[$((IDX/10))]}
    SYS=${SYSTEMS[$((IDX%10))]}
    RQ=1
    OUT=$RESULTS/rq_1_${SYS}_${STRAT}.csv
else 
    STRAT=random
    SYS=${SYSTEMS[$((IDX-60))]}
    RQ=2
    OUT=$RESULTS/rq_2_${SYS}_${STRAT}.csv
fi 

echo "Fit $IDX: Rq=$RQ Strategy=$STRAT System=$SYS written to $OUT"

python -u -c "from bindings.python.Interplay_ML.ML.Learn import run; run($RQ, '$SYS', '$STRAT', '$OUT')"