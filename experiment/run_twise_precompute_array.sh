#!/bin/bash
#SBATCH --job-name=twise_pre
#SBATCH --output=/scratch/miec00001/slurm_logs/twise_%A_%a.out
#SBATCH --time=1-02:59:59
#SBATCH --mem=256G
#SBATCH --constraint=kine
#SBATCH --partition=anywhere
#SBATCH --mail-user=miec00001@stud.uni-saarland.de
#SBATCH --mail-type=END,FAIL

set -euo pipefail

ROOT="/scratch/miec00001/Thesis/vara-feature"
EXP_DIR="$ROOT/experiment"
RUN_DIR="$ROOT/experiment_runs/twise_${SLURM_ARRAY_JOB_ID}_${SLURM_ARRAY_TASK_ID}"
OUT_DIR="$EXP_DIR/twise_precompute_results"
BIN="$ROOT/build/bin/greedy_twise_sampling"

mkdir -p "$RUN_DIR" "$OUT_DIR"
cd "$EXP_DIR"

LINE=$(sed -n "${SLURM_ARRAY_TASK_ID}p" twise_params.txt)

if [ -z "$LINE" ]; then
  echo "No parameters for task ${SLURM_ARRAY_TASK_ID}"
  exit 1
fi

SYSTEM_PATH_REL=$(echo "$LINE" | awk '{print $1}')
SYSTEM_NAME=$(echo "$LINE" | awk '{print $2}')
SOURCE_T=$(echo "$LINE" | awk '{print $3}')

SYSTEM_PATH="$ROOT/$SYSTEM_PATH_REL"
RAW_OUT="$RUN_DIR/${SYSTEM_NAME}_t${SOURCE_T}.csv"
FINAL_OUT="$OUT_DIR/${SLURM_ARRAY_TASK_ID}_${SYSTEM_NAME}_t${SOURCE_T}.csv"

if [ ! -x "$BIN" ]; then
  echo "Missing executable: $BIN"
  exit 1
fi

if [ ! -f "$SYSTEM_PATH" ]; then
  echo "Missing system file: $SYSTEM_PATH"
  exit 1
fi

echo "SYSTEM_NAME=$SYSTEM_NAME"
echo "SOURCE_T=$SOURCE_T"
echo "SYSTEM_PATH=$SYSTEM_PATH"
echo "RAW_OUT=$RAW_OUT"
echo "FINAL_OUT=$FINAL_OUT"

"$BIN" "$SYSTEM_PATH" "$SOURCE_T" "$RAW_OUT"

if [ ! -f "$RAW_OUT" ]; then
  echo "Expected output not created: $RAW_OUT"
  exit 1
fi

cp "$RAW_OUT" "$FINAL_OUT"

echo "done"