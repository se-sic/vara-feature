import pandas as pd
import numpy as np
from experiment_config import SamplingStrategy, SamplingStrategies, Metrics, Strength
from scipy import stats

def rho_strength(rho:float) -> Strength:
    tmp = abs(rho)
    if (tmp >= 0.95):
        return Strength.VERYSTRONG
    elif (tmp >= 0.80):
        return Strength.STRONG
    elif (tmp >= 0.60):
        return Strength.MODERATE
    elif (tmp >= 0.40):
        return Strength.WEAK
    return Strength.VERYWEAK

def interpret_row(row):
        if (row["mean_abs_rho"] >= 0.95):
            return "metrics are almost interchangeable"
        if (row["mean_abs_rho"] >= 0.80):
            return "metrics are strongly aligned"
        if (row["mean_abs_rho"] >= 0.60):
            return "metrics are moderately aligned"
        return "metrics capture meaningfully different behavior"

def main(output_csv):
    df = pd.read_csv(output_csv)
    mean = df.groupby(
    ["system", "strategy", "sample_size_source_t", "sample_size", "coverage_t", "metric"],
    as_index=False
    )["coverage"].mean()

    print(mean.head())

    results = []

    for strategy in SamplingStrategies:
    # for strategy in [SamplingStrategy.RANDOM]:
        for coverage_t in [1, 2, 3]:
            for source_t in [1, 2, 3]:
                subset = mean[
                    (mean["strategy"] == strategy.value) &
                    (mean["coverage_t"] == coverage_t) &
                    (mean["sample_size_source_t"] == source_t)
                ]
                seen_pairs = set()

                for metric1 in Metrics:
                    for metric2 in Metrics:
                        if metric1.value == metric2.value:
                            continue

                        pair_key = tuple(sorted([metric1.value, metric2.value]))

                        if pair_key in seen_pairs:
                            continue

                        seen_pairs.add(pair_key)

                        m1 = subset[subset["metric"] == metric1.value][["system", "coverage"]]
                        m2 = subset[subset["metric"] == metric2.value][["system", "coverage"]]

                        paired = m1.merge(m2, on="system", suffixes=("_m1", "_m2"))

                        if len(paired) < 2:
                            continue

                        rho, p_value = stats.spearmanr(
                            paired["coverage_m1"],
                            paired["coverage_m2"]
                        )

                        results.append({
                            "strategy": strategy.value,
                            "coverage_t": coverage_t,
                            "sample_size_source_t": source_t,
                            "metric_1": metric1.value,
                            "metric_2": metric2.value,
                            "rho": rho,
                            "rho_abs": abs(rho),
                            "rho_strength": rho_strength(rho),
                            "p_value": p_value,
                            "significant": p_value < 0.05,
                            "n_systems": len(paired),
                        })

    results_df = pd.DataFrame(results)
    results_df.to_csv("rq1_spearman_results.csv", index=False)

    summary = (
        results_df.groupby(["metric_1", "metric_2"], as_index=False).agg(
            mean_rho=("rho", "mean"),
            min_rho=("rho_min"),
            max_rho=("rho", "max"),
            mean_abs_rho=("rho_abs", "mean"),
            n_settings=("rho", "size"),
            n_strong_agreement=("rho_abs", lambda s: int((s >= 0.95).sum())),
            n_significant=("significant", "sum"),
        )
    )

    summary["interpretation"] = summary.apply(interpret_row, axis=1)
    summary = summary.sort_values(["mean_abs_rho", "mean_rho"], ascending=False)
    summary.to_csv("rq1_summary.csv", index=False)