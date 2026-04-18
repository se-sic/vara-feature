from pathlib import Path
from experiment_config import SamplingStrategy, SamplingStrategies, systems, Strength, effect_strength, kruskal_epsilon_squared, relation_label, Relations
from scipy import stats

import pandas as pd
import subprocess
import scikit_posthocs as sp

def trend_direction(mw, mm, ms, mv) -> Relations:
    if mw >= mm >= ms >= mv:
        return Relations.DECREASING.value
    if mw <= mm <= ms <= mv:
        return Relations.INCREASING.value
    return Relations.NONMONOTONIC.value

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
                    subset["constraint_level"] == Strength.WEAK.value
                ]["coverage"]

                moderate = subset[
                    subset["constraint_level"] == Strength.MODERATE.value
                ]["coverage"]

                strong = subset[
                    subset["constraint_level"] == Strength.STRONG.value
                ]["coverage"]

                veryStrong = subset[
                    subset["constraint_level"] == Strength.VERYSTRONG.value
                ]["coverage"]

                if len(weak) == 0 or len(moderate) == 0 or len(strong) == 0 or len(veryStrong) == 0:
                    continue

                kw = stats.kruskal(weak, moderate, strong, veryStrong)

                n_total = len(weak) + len(moderate) + len(strong) + len(veryStrong)
                eps_sq = kruskal_epsilon_squared(kw.statistic, n_total, 4)

                med_weak = float(weak.median())
                med_moderate = float(moderate.median())
                med_strong = float(strong.median())
                med_verystrong = float(veryStrong.median())

                means = {
                    Strength.WEAK.value: med_weak,
                    Strength.MODERATE.value: med_moderate,
                    Strength.STRONG.value: med_strong,
                    Strength.VERYSTRONG.value: med_verystrong,
                }

                best_group = max(means, key=means.get)
                worst_group = min(means, key=means.get)
                weak_vs_verystrong_gap = med_weak - med_verystrong

                dunn_weak_moderate = None
                dunn_weak_strong = None
                dunn_weak_verystrong = None
                dunn_moderate_strong = None
                dunn_moderate_verystrong = None
                dunn_strong_verystrong = None

                if kw.pvalue < 0.05:
                    grouped = subset[
                        subset["constraint_level"].isin([
                            Strength.WEAK.value,
                            Strength.MODERATE.value,
                            Strength.STRONG.value,
                            Strength.VERYSTRONG.value,
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

                    dunn_weak_moderate = get_dunn(Strength.WEAK.value, Strength.MODERATE.value)
                    dunn_weak_strong = get_dunn(Strength.WEAK.value, Strength.STRONG.value)
                    dunn_weak_verystrong = get_dunn(Strength.WEAK.value, Strength.VERYSTRONG.value)
                    dunn_moderate_strong = get_dunn(Strength.MODERATE.value, Strength.STRONG.value)
                    dunn_moderate_verystrong = get_dunn(Strength.MODERATE.value, Strength.VERYSTRONG.value)
                    dunn_strong_verystrong = get_dunn(Strength.STRONG.value, Strength.VERYSTRONG.value)

                pairwise_summary = "; ".join([
                    relation_label(dunn_weak_moderate, Strength.WEAK.value, med_weak, Strength.MODERATE.value, med_moderate),
                    relation_label(dunn_weak_strong, Strength.WEAK.value, med_weak, Strength.STRONG.value, med_strong),
                    relation_label(dunn_weak_verystrong, Strength.WEAK.value, med_weak, Strength.VERYSTRONG.value, med_verystrong),
                    relation_label(dunn_moderate_strong, Strength.MODERATE.value, med_moderate, Strength.STRONG.value, med_strong),
                    relation_label(dunn_moderate_verystrong, Strength.MODERATE.value, med_moderate, Strength.VERYSTRONG.value, med_verystrong),
                    relation_label(dunn_strong_verystrong, Strength.STRONG.value,med_strong, Strength.VERYSTRONG.value, med_verystrong),
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
                    "median_weak": med_weak,
                    "median_moderate": med_moderate,
                    "median_strong": med_strong,
                    "median_verystrong": med_verystrong,
                    "best_group_by_median": best_group,
                    "worst_group_by_median": worst_group,
                    "weak_minus_verystrong": weak_vs_verystrong_gap,
                    "trend_direction": trend_direction(med_weak, med_moderate, med_strong, med_verystrong),
                    "dunn_weak_moderate": dunn_weak_moderate,
                    "dunn_weak_strong": dunn_weak_strong,
                    "dunn_weak_verystrong": dunn_weak_verystrong,
                    "dunn_moderate_strong": dunn_moderate_strong,
                    "dunn_moderate_verystrong": dunn_moderate_verystrong,
                    "dunn_strong_verystrong": dunn_strong_verystrong,
                    "pairwise_summary": pairwise_summary,
                })
    results_df = pd.DataFrame(results)
    results_df.to_csv("rq3_kruskal_dunn_results.csv", index=False)

    summary = (
        results_df.groupby("metric", as_index=False)
        .agg(
            n_settings=("metric", "size"),
            n_kw_significant=("kw_significant", "sum"),
            mean_epsilon_squared=("epsilon_squared", "mean"),
            mean_weak_minus_verystrong=("weak_minus_verystrong", "mean"),
            n_decreasing=("trend_direction", lambda s: int((s == Relations.DECREASING.value).sum())),
            n_non_monotonic=("trend_direction", lambda s: int((s == Relations.NONMONOTONIC.value).sum())),
            n_increasing=("trend_direction", lambda s: int((s == Relations.INCREASING.value).sum())),
        )
    )
    summary.to_csv("rq3_summary.csv", index=False)