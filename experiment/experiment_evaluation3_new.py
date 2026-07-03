from pathlib import Path
from experiment_config import systems
from scipy import stats

import pandas as pd
import subprocess


def main(output_csv):
    cov = pd.read_csv(output_csv)

    constraint_ratio_csv = Path("constraint_ratios.csv").resolve()
    strategy_path = Path("../build/bin/compute_constraint_ratio").resolve()

    if constraint_ratio_csv.exists():
        constraint_ratio_csv.unlink()

    for system_path, system_name in systems:
        subprocess.run(
            [
                str(strategy_path),
                str(system_path),
                str(system_name),
                str(constraint_ratio_csv),
            ],
            check=True,
        )

    ratios = pd.read_csv(constraint_ratio_csv)[["system", "log10_ratio"]]

    df = cov.merge(ratios, on="system", how="left")

    RESULTS_DIR = Path("results")
    RESULTS_DIR.mkdir(exist_ok=True)


    raw = (
        df.groupby(["metric", "coverage_t", "sample_size_source_t", "system"], as_index=False)
        .agg(coverage=("coverage", "mean"), log10_ratio=("log10_ratio", "first"))
    )
    raw["setting"] = raw.apply(
        lambda r: f"({int(r['coverage_t'])},{int(r['sample_size_source_t'])})", axis=1
    )
    raw.to_csv(RESULTS_DIR / "rq3_raw_values_new.csv", index=False)

    results = []
    for metric_name in sorted(df["metric"].unique()):
        for coverage_t in [1, 2, 3]:
            for source_t in [1, 2, 3]:
                g = raw[
                    (raw["metric"] == metric_name)
                    & (raw["coverage_t"] == coverage_t)
                    & (raw["sample_size_source_t"] == source_t)
                ][["system", "coverage", "log10_ratio"]].dropna()

                if g.shape[0] < 3:
                    print(
                        "Skipping (too few systems):",
                        metric_name, (coverage_t, source_t), "n =", g.shape[0],
                    )
                    continue

                rho, p = stats.spearmanr(g["log10_ratio"], g["coverage"])

                results.append({
                    "metric": metric_name,
                    "coverage_t": coverage_t,
                    "sample_size_source_t": source_t,
                    "setting": f"({coverage_t},{source_t})",
                    "n_systems": int(g.shape[0]),
                    "spearman_rho": rho,
                    "pvalue": p,
                    "significant": p < 0.05,
                })

    results_df = pd.DataFrame(results)
    results_df.to_csv(RESULTS_DIR / "rq3_spearman_results_new.csv", index=False)

    summary = (
        results_df.groupby("metric", as_index=False)
        .agg(
            n_settings=("metric", "size"),
            n_significant=("significant", "sum"),
            mean_rho=("spearman_rho", "mean"),
            min_rho=("spearman_rho", "min"),
            max_rho=("spearman_rho", "max"),
        )
    )
    summary.to_csv(RESULTS_DIR / "rq3_summary_new.csv", index=False)


if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        raise SystemExit(f"Usage: python {__file__} <results.csv>")
    main(sys.argv[1])