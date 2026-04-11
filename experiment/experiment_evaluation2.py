from experiment_config import SamplingStrategy
from scipy import stats

import pandas as pd
import scikit_posthocs as sp


def main(output_csv):
    df = pd.read_csv(output_csv)

    results = []

    for metric_name in df["metric"].unique():
        for coverage_t in [1, 2, 3]:
            for source_t in [1, 2, 3]:
                subset = df[
                    (df["metric"] == metric_name) &
                    (df["coverage_t"] == coverage_t) &
                    (df["sample_size_source_t"] == source_t)
                ]

                if subset.empty:
                    continue

                random = subset[
                    subset["strategy"] == SamplingStrategy.RANDOM.value
                ]["coverage"]

                solver = subset[
                    subset["strategy"] == SamplingStrategy.SOLVER.value
                ]["coverage"]

                distance = subset[
                    subset["strategy"] == SamplingStrategy.DISTANCE.value
                ]["coverage"]

                if len(random) == 0 or len(solver) == 0 or len(distance) == 0:
                    continue

                kw = stats.kruskal(random, solver, distance)

                dunn_random_solver = None
                dunn_random_distance = None
                dunn_solver_distance = None

                if kw.pvalue < 0.05:
                    grouped = subset[
                        subset["strategy"].isin([
                            SamplingStrategy.RANDOM.value,
                            SamplingStrategy.SOLVER.value,
                            SamplingStrategy.DISTANCE.value,
                        ])
                    ][["strategy", "coverage"]]

                    dunn = sp.posthoc_dunn(
                        grouped,
                        val_col="coverage",
                        group_col="strategy",
                        p_adjust="holm"
                    )

                    dunn_random_solver = dunn.loc["random", "solver"]
                    dunn_random_distance = dunn.loc["random", "distance"]
                    dunn_solver_distance = dunn.loc["solver", "distance"]

                results.append({
                    "metric": metric_name,
                    "coverage_t": coverage_t,
                    "sample_size_source_t": source_t,
                    "kw_statistic": kw.statistic,
                    "kw_pvalue": kw.pvalue,
                    "dunn_random_solver": dunn_random_solver,
                    "dunn_random_distance": dunn_random_distance,
                    "dunn_solver_distance": dunn_solver_distance,
                })

    results_df = pd.DataFrame(results)
    results_df.to_csv("rq2_kruskal_dunn_results.csv", index=False)