#!/usr/bin/env python3
from pathlib import Path
import pandas as pd

ROOT = Path("/scratch/miec00001/Thesis/vara-feature")
RUNS_DIR = ROOT / "experiment_runs"
OUT_DIR = ROOT / "experiment"

def main() -> None:
    result_files = sorted(RUNS_DIR.glob("job_*/results.csv"))

    if not result_files:
        raise SystemExit("No results.csv files found in experiment_runs/job_*/")

    dfs = []
    bad_files = []

    for result_file in result_files:
        try:
            if result_file.stat().st_size == 0:
                bad_files.append((result_file, "empty"))
                continue

            df = pd.read_csv(result_file)
            if df.empty:
                bad_files.append((result_file, "no rows"))
                continue

            df["source_result_file"] = str(result_file)
            dfs.append(df)

        except Exception as exc:
            bad_files.append((result_file, str(exc)))

    if not dfs:
        raise SystemExit("No readable non-empty result files found.")

    merged = pd.concat(dfs, ignore_index=True)

    out_file = OUT_DIR / "results.csv"
    merged.to_csv(out_file, index=False)

    print(f"Merged {len(dfs)} result files into {out_file}")
    print(f"Total rows: {len(merged)}")

    if bad_files:
        bad_file_report = OUT_DIR / "merge_skipped_files.txt"
        with bad_file_report.open("w", encoding="utf-8") as fh:
            for path, reason in bad_files:
                fh.write(f"{path}\t{reason}\n")
        print(f"Skipped {len(bad_files)} files. See {bad_file_report}")

if __name__ == "__main__":
    main()
