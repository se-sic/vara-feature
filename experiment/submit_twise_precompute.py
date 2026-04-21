#!/usr/bin/env python3
from itertools import product
from pathlib import Path
import subprocess
import re

ROOT = Path("/scratch/miec00001/Thesis/vara-feature/experiment")
PARAMS = ROOT / "twise_params.txt"

systems = [
    ("Random_Sampler/examples/FeatureModel/7z.xml", "7z"),
    ("Random_Sampler/examples/FeatureModel/BerkeleyDBC.xml", "BerkeleyDBC"),
    ("Random_Sampler/examples/FeatureModel/Dune.xml", "Dune"),
    ("Random_Sampler/examples/FeatureModel/Hipacc.xml", "Hipacc"),
    ("Random_Sampler/examples/FeatureModel/JavaGC.xml", "JavaGC"),
    ("Random_Sampler/examples/FeatureModel/LLVM.xml", "LLVM"),
    ("Random_Sampler/examples/FeatureModel/lrzip.xml", "lrzip"),
    ("Random_Sampler/examples/FeatureModel/Polly.xml", "Polly"),
    ("Random_Sampler/examples/FeatureModel/VP9.xml", "VP9"),
    ("Random_Sampler/examples/FeatureModel/x264.xml", "x264"),
    ("Random_Sampler/examples/FeatureModel/AJStats.xml", "AJStats"),
    ("Random_Sampler/examples/FeatureModel/Curl.xml", "Curl"),
    ("Random_Sampler/examples/FeatureModel/HSMGP.xml", "HSMGP"),
    ("Random_Sampler/examples/FeatureModel/HSQLDB.xml", "HSQLDB"),
    ("Random_Sampler/examples/FeatureModel/HyTeG.xml", "HyTeG"),
    ("Random_Sampler/examples/FeatureModel/PKJab.xml", "PKJab"),
    ("Random_Sampler/examples/FeatureModel/SQLite.xml", "SQLite"),
    ("Random_Sampler/examples/FeatureModel/TriMesh.xml", "TriMesh"),
    ("Random_Sampler/examples/FeatureModel/WGet.xml", "WGet"),
    ("Random_Sampler/examples/FeatureModel/clasp.xml", "clasp"),
    ("Random_Sampler/examples/FeatureModel/z3.xml", "z3"),
]

source_ts = [1, 2, 3]


def main() -> None:
    lines = []

    for (system_path_str, system_name), source_t in product(systems, source_ts):
        lines.append(f"{system_path_str} {system_name} {source_t}")

    PARAMS.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {len(lines)} tasks to {PARAMS}")

    result = subprocess.run(
        ["sbatch", f"--array=1-{len(lines)}", "run_twise_precompute_array.sh"],
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
        print(f"Check with: squeue -u $USER | grep {job_id}")
        print(f"Logs: ls /scratch/miec00001/slurm_logs/twise_{job_id}_*.out")


if __name__ == "__main__":
    main()