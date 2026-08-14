#!/bin/bash
#SBATCH --job-name=fitting
#SBATCH --output=/scratch/mani00001/slurm_logs/fitting_file_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=200G
#SBATCH --partition=anywhere
#SBATCH --mail-user=mani00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL
#SBATCH --array=0-119
#SBATCH --cpus-per-task=4

set -e
REPO=$(cd "$SLURM_SUBMIT_DIR/../../../.." && pwd)
cd "$REPO"
source .venv/bin/activate
export PYTHONPATH="$REPO/build/bindings/python/vara-feature:$PYTHONPATH"

SYSTEMS=(7z BerkeleyDBC Dune Hippacc Irzip JavaGC LLVM Polly VP9 x264)
RQ1_STRATS=(random twise sbs rsbs dbs ddbs)
PROPS=(0.01 0.02 0.05 0.1 0.15 0.2)
RESULTS="$REPO/bindings/python/Interplay_ML/Results"
PARTIALS="$RESULTS/partials"
IDX=$SLURM_ARRAY_TASK_ID

if [ ! -d "$REPO/bindings/python/Interplay_ML/Samples" ]; then 
    echo "No samples for fitting given" >&2
    exit 1
fi

if [ "$IDX" -lt 60 ]; then 
    STRAT=${RQ1_STRATS[$((IDX/10))]}
    SYS=${SYSTEMS[$((IDX%10))]}
    OUT="$RESULTS/rq_1_${SYS}_${STRAT}.csv"

    echo "Fit RQ1: $SYS $STRAT"
    python -m bindings.python.Interplay_ML.ML.Learn fit 1 "$SYS" "$STRAT" "$OUT"
else
    NIDX=$((IDX-60))
    STRAT=random
    SYS=${SYSTEMS[$((NIDX/6))]}
    PROP=${PROPS[$((NIDX%6))]}
    OUT="$PARTIALS/rq_2_${SYS}_random_${PROP}.csv"

    echo "Fit RQ2: $SYS $PROP"
    python -m bindings.python.Interplay_ML.ML.Learn fit 2 "$SYS" random "$OUT" --only_prop "$PROP"
fi