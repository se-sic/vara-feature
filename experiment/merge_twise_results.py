#!/usr/bin/env python3
from pathlib import Path
import csv

ROOT = Path("/scratch/miec00001/Thesis/vara-feature/experiment")
IN_DIR = ROOT / "twise_precompute_results"
OUT_CSV = ROOT / "twise_sample_sizes.csv"


def main() -> None:
    files = sorted(IN_DIR.glob("*.csv"))
    if not files:
        raise SystemExit(f"No CSV files found in {IN_DIR}")

    rows: list[dict[str, str]] = []

    for file in files:
        with file.open("r", encoding="utf-8", newline="") as fh:
            reader = csv.DictReader(fh)
            first_row = next(reader, None)
            if first_row is None:
                raise SystemExit(f"Empty CSV: {file}")

            if "system" not in first_row or "sample_size" not in first_row:
                raise SystemExit(f"Unexpected columns in {file}: {list(first_row.keys())}")

            rows.append(
                {
                    "system_name": first_row["system"],
                    "sample_size": first_row["sample_size"],
                    "source_file": file.name,
                }
            )

    def extract_source_t(source_file: str) -> int:
        marker = "_t"
        start = source_file.rfind(marker)
        if start == -1:
            raise ValueError(f"Cannot parse source_t from {source_file}")
        start += len(marker)
        end = source_file.find(".csv", start)
        return int(source_file[start:end])

    for row in rows:
        row["source_t"] = str(extract_source_t(row["source_file"]))

    rows.sort(key=lambda r: (r["system_name"].lower(), int(r["source_t"])))

    with OUT_CSV.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(
            fh,
            fieldnames=["system_name", "source_t", "sample_size"],
        )
        writer.writeheader()
        for row in rows:
            writer.writerow(
                {
                    "system_name": row["system_name"],
                    "source_t": row["source_t"],
                    "sample_size": row["sample_size"],
                }
            )

    print(f"Wrote {OUT_CSV} with {len(rows)} rows")


if __name__ == "__main__":
    main()