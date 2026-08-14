#!/bin/bash
#SBATCH --job-name=tuning
#SBATCH --output=/scratch/mani00001/slurm_logs/tuning_%A_%a.out
#SBATCH --constraint=kine
#SBATCH --time=24:00:00
#SBATCH --mem=200G
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
SYS=${SYSTEMS[$SLURM_ARRAY_TASK_ID]}

if [ ! -d "$REPO/bindings/python/Interplay_ML/Samples" ]; then
    echo "Tuning Sample not found in /Samples" >&2
    exit 1
fi

echo "Tuning $SLURM_ARRAY_TASK_ID: $SYS"
#python -m bindings.python.Interplay_ML.ML.Tune system "$SYS"
python -X faulthandler -u -m bindings.python.Interplay_ML.ML.Tune system "$SYS"