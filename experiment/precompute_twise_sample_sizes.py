#!/usr/bin/env python3
from pathlib import Path
import csv
import subprocess
import sys

ROOT = Path("/scratch/miec00001/Thesis/vara-feature")
EXP_DIR = ROOT / "experiment"
OUT_CSV = EXP_DIR / "twise_sample_sizes.csv"

SYSTEMS = [
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

SOURCE_TS = [1, 2, 3]

# Change this if the actual binary name/path is different.
GREEDY_TWISE_BIN = ROOT / "build" / "bin" / "greedy_twise_sampling"


def compute_sample_size(system_path: Path, source_t: int, work_dir: Path) -> int:
    output_csv = work_dir / f"twise_{system_path.stem}_t{source_t}.csv"
    if output_csv.exists():
        output_csv.unlink()

    subprocess.run(
        [
            str(GREEDY_TWISE_BIN),
            str(system_path),
            str(source_t),
            str(output_csv),
        ],
        check=True,
    )

    if not output_csv.exists():
        raise RuntimeError(f"Expected output CSV was not created: {output_csv}")

    with output_csv.open("r", encoding="utf-8", newline="") as fh:
        reader = csv.DictReader(fh)
        first_row = next(reader, None)
        if first_row is None:
            raise RuntimeError(f"CSV is empty: {output_csv}")
        if "sample_size" not in first_row:
            raise RuntimeError(f"'sample_size' column missing in: {output_csv}")
        return int(first_row["sample_size"])


def main() -> int:
    if not GREEDY_TWISE_BIN.exists():
        print(f"ERROR: binary not found: {GREEDY_TWISE_BIN}", file=sys.stderr)
        return 1

    tmp_dir = EXP_DIR / "tmp_twise_precompute"
    tmp_dir.mkdir(parents=True, exist_ok=True)

    rows: list[dict[str, str | int]] = []

    for rel_path, system_name in SYSTEMS:
        system_path = (ROOT / rel_path).resolve()
        if not system_path.exists():
            print(f"ERROR: system file not found: {system_path}", file=sys.stderr)
            return 1

        for source_t in SOURCE_TS:
            print(f"Computing sample size for {system_name}, t={source_t} ...")
            sample_size = compute_sample_size(system_path, source_t, tmp_dir)
            rows.append(
                {
                    "system_name": system_name,
                    "system_path": rel_path,
                    "source_t": source_t,
                    "sample_size": sample_size,
                }
            )
            print(f"  -> {sample_size}")

    with OUT_CSV.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(
            fh,
            fieldnames=["system_name", "system_path", "source_t", "sample_size"],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {OUT_CSV}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())