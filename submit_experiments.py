#!/usr/bin/env python3
from itertools import product
from pathlib import Path
import subprocess
import re

ROOT = Path("/scratch/miec00001/Thesis/vara-feature/experiment")
PARAMS = ROOT / "experiment_params.txt"

def get_sample_size_from_twise(system_path: Path, source_t: int):
    sample_sizes = {
        "7z": [39, 600, 4091],
        "BerkeleyDBC": [15, 97, 343],
        "Dune": [25, 265, 1071],
        "Hipacc": [50, 843, 4601],
        "JavaGC": [32, 468, 3504],
        "LLVM": [11, 55, 165],
        "Polly": [28, 345, 2172],
        "VP9": [31, 483, 3893],
        "lrzip": [18, 90, 178],
        "x264": [12, 65, 212],
    }

    system_name = system_path.stem
    return sample_sizes[system_name][source_t - 1]

systems = [
    ("Random_Sampler/examples/FeatureModel/7z.xml", "7z"),
]

strategies = ["random", "solver", "distance"]
source_ts = [1, 2, 3]
iterations = range(1, 2)

lines = []

for system_path_str, system_name in systems:
    system_path = Path(system_path_str)

    for strategy, source_t, iteration in product(strategies, source_ts, iterations):
        sample_size = 10
        lines.append(
            f"{system_path_str} {system_name} {strategy} {source_t} {sample_size} {iteration}"
        )

PARAMS.write_text("\n".join(lines) + "\n", encoding="utf-8")
print(f"Wrote {len(lines)} tasks to {PARAMS}")

result = subprocess.run(
    ["sbatch", f"--array=1-{len(lines)}", "run_experiment_array.sh"],
    cwd=ROOT,
    text=True,
    capture_output=True,
    check=True,
)

print(result.stdout.strip())

match = re.search(r"Submitted batch job (\d+)", result.stdout)
if match:
    job_id = match.group(1)
    print(f"Job array ID: {job_id}")
    print(f"Check with: squeue | grep {job_id}")
    print(f"Logs: ls /scratch/miec00001/slurm_logs/exp_{job_id}_*.out")
