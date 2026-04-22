#!/usr/bin/env python3
from pathlib import Path
import csv

ROOT = Path("/scratch/miec00001/Thesis/vara-feature")
TWISE_SAMPLE_SIZES_CSV = ROOT / "experiment" / "twise_sample_sizes.csv"


def get_sample_size_from_twise(system_path: Path, source_t: int) -> int:
    system_name = system_path.stem

    if not TWISE_SAMPLE_SIZES_CSV.exists():
        raise FileNotFoundError(
            f"Missing sample-size table: {TWISE_SAMPLE_SIZES_CSV}"
        )

    with TWISE_SAMPLE_SIZES_CSV.open("r", encoding="utf-8", newline="") as fh:
        reader = csv.DictReader(fh)
        for row in reader:
            if row["system_name"] == system_name and int(row["source_t"]) == source_t:
                return int(row["sample_size"])

    raise ValueError(
        f"No sample size found for system={system_name}, source_t={source_t}"
    )