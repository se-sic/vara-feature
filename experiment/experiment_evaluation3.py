from pathlib import Path
from experiment_config import SamplingStrategy, SamplingStrategies, systems, ConstraintLevel
from scipy import stats

import pandas as pd
import subprocess
import scikit_posthocs as sp


def main(output_csv):
    df1 = pd.read_csv(output_csv)
    
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

    df2 = pd.read_csv(constraint_ratio_csv)

    df = df1.merge(df2, on="system", how="left")

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

                weak = subset[
                    subset["constraint_level"] == ConstraintLevel.WEAK.value
                ]["coverage"]

                moderate = subset[
                    subset["constraint_level"] == ConstraintLevel.MODERATE.value
                ]["coverage"]

                strong = subset[
                    subset["constraint_level"] == ConstraintLevel.STRONG.value
                ]["coverage"]

                veryStrong = subset[
                    subset["constraint_level"] == ConstraintLevel.VERYSTRONG.value
                ]["coverage"]

                if len(weak) == 0 or len(moderate) == 0 or len(strong) == 0 or len(veryStrong) == 0:
                    continue

                kw = stats.kruskal(weak, moderate, strong, veryStrong)

                dunn_weak_moderate = None
                dunn_weak_strong = None
                dunn_weak_verystrong = None
                dunn_moderate_strong = None
                dunn_moderate_verystrong = None
                dunn_strong_verystrong = None

                if kw.pvalue < 0.05:
                    grouped = subset[
                        subset["constraint_level"].isin([
                            ConstraintLevel.WEAK.value,
                            ConstraintLevel.MODERATE.value,
                            ConstraintLevel.STRONG.value,
                            ConstraintLevel.VERYSTRONG.value,
                        ])
                    ][["constraint_level", "coverage"]]

                    dunn = sp.posthoc_dunn(
                        grouped,
                        val_col="coverage",
                        group_col="constraint_level",
                        p_adjust="holm"
                    )

                    labels = set(dunn.index)

                    def get_dunn(a, b):
                        return dunn.loc[a, b] if {a, b}.issubset(labels) else None

                    dunn_weak_moderate = get_dunn(ConstraintLevel.WEAK.value, ConstraintLevel.MODERATE.value)
                    dunn_weak_strong = get_dunn(ConstraintLevel.WEAK.value, ConstraintLevel.STRONG.value)
                    dunn_weak_verystrong = get_dunn(ConstraintLevel.WEAK.value, ConstraintLevel.VERYSTRONG.value)
                    dunn_moderate_strong = get_dunn(ConstraintLevel.MODERATE.value, ConstraintLevel.STRONG.value)
                    dunn_moderate_verystrong = get_dunn(ConstraintLevel.MODERATE.value, ConstraintLevel.VERYSTRONG.value)
                    dunn_strong_verystrong = get_dunn(ConstraintLevel.STRONG.value, ConstraintLevel.VERYSTRONG.value)

                results.append({
                "metric": metric_name,
                "coverage_t": coverage_t,
                "sample_size_source_t": source_t,
                "kw_statistic": kw.statistic,
                "kw_pvalue": kw.pvalue,
                "dunn_weak_moderate": dunn_weak_moderate,
                "dunn_weak_strong": dunn_weak_strong,
                "dunn_weak_verystrong": dunn_weak_verystrong,
                "dunn_moderate_strong": dunn_moderate_strong,
                "dunn_moderate_verystrong": dunn_moderate_verystrong,
                "dunn_strong_verystrong": dunn_strong_verystrong,
                })
    results_df = pd.DataFrame(results)
    results_df.to_csv("rq3_kruskal_dunn_results.csv", index=False)