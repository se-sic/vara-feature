#!/bin/bash

set -e
cd /scratch/mani00001/vara-feature-1
source .venv/bin/activate
export PYTHONPATH="/scratch/mani00001/vara-feature-1/build/bindings/python/vara-feature:$PYTHONPATH"

SYS=$1
T=$2
SIZE=$(./build/bin/random_sampler "Random_Sampler/examples/${SYS}.xml" tsizes "$T" | tail -1)
echo "${SYS},${T},${SIZE}"