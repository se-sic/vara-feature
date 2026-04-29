from experiment_config import SamplingStrategy, effect_strength, kruskal_epsilon_squared, relation_label
from scipy import stats
from pathlib import Path

import pandas as pd
import scikit_posthocs as sp

def overall_winner(row):
        wins = {
            SamplingStrategy.RANDOM.value: row["random_wins"],
            SamplingStrategy.SOLVER.value: row["solver_wins"],
            SamplingStrategy.DISTANCE.value: row["distance_wins"],
        }
        return max(wins, key=wins.get)

def main(output_csv):
    df = pd.read_csv(output_csv)

    raw_df = df.copy()
    raw_df["setting"] = raw_df.apply(
        lambda row: f"({int(row['coverage_t'])},{int(row['sample_size_source_t'])})",
        axis=1,
    )
    RESULTS_DIR = Path("results")
    RESULTS_DIR.mkdir(exist_ok=True)

    raw_df.to_csv(RESULTS_DIR / "rq2_raw_values.csv", index=False)

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

                n_total = len(random) + len(solver) + len(distance)
                eps_sq = kruskal_epsilon_squared(kw.statistic, n_total, 3)

                med_random = float(random.median())
                med_solver = float(solver.median())
                med_distance = float(distance.median())

                mean_random = float(random.mean())
                mean_solver = float(solver.mean())
                mean_distance = float(distance.mean())

                strategy_medians = {
                    SamplingStrategy.RANDOM.value: med_random,
                    SamplingStrategy.SOLVER.value: med_solver,
                    SamplingStrategy.DISTANCE.value: med_distance,
                }

                best_strategy = max(strategy_medians, key=strategy_medians.get)
                worst_strategy = min(strategy_medians, key=strategy_medians.get)
                median_gap_best_vs_worst = strategy_medians[best_strategy] -strategy_medians[worst_strategy]

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

                    dunn_random_solver = dunn.loc[SamplingStrategy.RANDOM.value, SamplingStrategy.SOLVER.value]
                    dunn_random_distance = dunn.loc[SamplingStrategy.RANDOM.value, SamplingStrategy.DISTANCE.value]
                    dunn_solver_distance = dunn.loc[SamplingStrategy.SOLVER.value, SamplingStrategy.DISTANCE.value]

                pairwise_summary = "; ".join([
                    relation_label(dunn_random_solver, SamplingStrategy.RANDOM.value, med_random, SamplingStrategy.SOLVER.value, med_solver),
                    relation_label(dunn_random_distance, SamplingStrategy.RANDOM.value, med_random, SamplingStrategy.DISTANCE.value, med_distance),
                    relation_label(dunn_solver_distance, SamplingStrategy.SOLVER.value, med_solver, SamplingStrategy.DISTANCE.value, med_distance),
                ])

                results.append({
                    "metric": metric_name,
                    "coverage_t": coverage_t,
                    "sample_size_source_t": source_t,
                    "kw_statistic": kw.statistic,
                    "kw_pvalue": kw.pvalue,
                    "kw_significant": kw.pvalue < 0.05,
                    "epsilon_squared": eps_sq,
                    "effect_strength": effect_strength(eps_sq),
                    "median_random": med_random,
                    "median_solver": med_solver,
                    "median_distance": med_distance,
                    "mean_random": mean_random,
                    "mean_solver": mean_solver,
                    "mean_distance": mean_distance,
                    "best_strategy_by_median": best_strategy,
                    "worst_strategy_by_median": worst_strategy,
                    "median_gap_best_vs_worst": median_gap_best_vs_worst,
                    "dunn_random_solver": dunn_random_solver,
                    "dunn_random_distance": dunn_random_distance,
                    "dunn_solver_distance": dunn_solver_distance,
                    "pairwise_summary": pairwise_summary,
                })

    RESULTS_DIR = Path("results")
    RESULTS_DIR.mkdir(exist_ok=True)

    results_df = pd.DataFrame(results)
    results_df.to_csv(RESULTS_DIR / "rq2_kruskal_dunn_results.csv", index=False)

    summary = (results_df.groupby("metric", as_index=False)
               .agg(n_settings=("metric", "size"),
                    n_kw=("kw_significant", "sum"),
                    mean_epsilon_squared=("epsilon_squared", "mean"),
                    random_wins=("best_strategy_by_median", lambda s: int((s == "random").sum())),
                    solver_wins=("best_strategy_by_median", lambda s: int((s == "solver").sum())),
                    distance_wins=("best_strategy_by_median", lambda s: int((s == "distance").sum())),
                    mean_gap_best_vs_worst=("median_gap_best_vs_worst", "mean"),
                    )
              )
    summary["overall_best_strategy"] = summary.apply(overall_winner, axis=1)
    summary.to_csv(RESULTS_DIR / "rq2_summary.csv", index=False)

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 2:
        raise SystemExit(f"Usage: python {__file__} <results.csv>")
    main(sys.argv[1])