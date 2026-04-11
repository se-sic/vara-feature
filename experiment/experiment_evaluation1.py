import pandas as pd
import numpy as np
from experiment_config import SamplingStrategy, SamplingStrategies, Metrics
from scipy import stats

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
                            "p_value": p_value,
                            "n_systems": len(paired),
                        })
    results_df = pd.DataFrame(results)
    results_df.to_csv("rq1_spearman_results.csv", index=False)