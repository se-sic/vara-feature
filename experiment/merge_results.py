#!/usr/bin/env python3
from pathlib import Path
import pandas as pd

ROOT = Path("/scratch/miec00001/Thesis/vara-feature")
RUNS_DIR = ROOT / "experiment_runs"
OUT_CSV = ROOT / "experiment" / "results.csv"


def main() -> None:
    csv_files = sorted(RUNS_DIR.glob("job_*_*/results.csv"))

    if not csv_files:
        raise FileNotFoundError(f"No result CSVs found under {RUNS_DIR}")

    dfs = []
    for csv_file in csv_files:
        df = pd.read_csv(csv_file)
        df["source_file"] = str(csv_file)
        dfs.append(df)

    merged = pd.concat(dfs, ignore_index=True)

    merged = merged.drop_duplicates(
        subset=[
            "system",
            "strategy",
            "sample_size_source_t",
            "sample_size",
            "coverage_t",
            "run",
            "metric",
        ],
        keep="last",
    )

    merged.to_csv(OUT_CSV, index=False)
    print(f"Merged {len(csv_files)} files into {OUT_CSV}")
    print(f"Rows: {len(merged)}")


if __name__ == "__main__":
    main()