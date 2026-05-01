#!/usr/bin/env python3

from pathlib import Path
from experiment_config import Strength, SamplingStrategy
from typing import cast
from matplotlib.axes import Axes
from matplotlib.patches import Rectangle
from matplotlib.colors import LinearSegmentedColormap, ListedColormap, BoundaryNorm, to_rgb

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import numpy.typing as npt


DATA_DIR = Path("results")
OUT_DIR = Path("plots")
OUT_DIR.mkdir(exist_ok=True)

RQ1_RESULTS = DATA_DIR / "rq1_spearman_results.csv"
RQ1_SUMMARY = DATA_DIR / "rq1_summary.csv"
RQ1_RAW = DATA_DIR / "rq1_raw_values.csv"
RQ2_RESULTS = DATA_DIR / "rq2_kruskal_dunn_results.csv"
RQ2_SUMMARY = DATA_DIR / "rq2_summary.csv"
RQ2_RAW = DATA_DIR / "rq2_raw_values.csv"
RQ3_RESULTS = DATA_DIR / "rq3_kruskal_dunn_results.csv"
RQ3_SUMMARY = DATA_DIR / "rq3_summary.csv"
RQ3_RAW = DATA_DIR / "rq3_raw_values.csv"


SETTING_ORDER = [
    (1, 1), (1, 2), (1, 3),
    (2, 1), (2, 2), (2, 3),
    (3, 1), (3, 2), (3, 3),
]

CONSTRAINT_ORDER = [Strength.WEAK.value, Strength.MODERATE.value, Strength.STRONG.value, Strength.VERYSTRONG.value]

STRATEGY_COLORS = {
    SamplingStrategy.RANDOM.value: "#4477AA",   # blue
    SamplingStrategy.DISTANCE.value: "#228833", # green
    SamplingStrategy.SOLVER.value: "#AA3377",   # purple/magenta
}

CONSTRAINT_COLORS = {
    Strength.WEAK.value: "#4477AA",
    Strength.MODERATE.value: "#66CCEE",
    Strength.STRONG.value: "#228833",
    Strength.VERYSTRONG.value: "#AA3377",
}

def get_value_enum(value: object) -> str:
    text = str(value)
    if "." in text:
        text = text.split(".")[-1]
    text = text.replace("_", " ")
    return text.lower()

def get_value_enum_column(df: pd.DataFrame, column: str) -> pd.DataFrame:
    if column not in df.columns:
        return df
    out = df.copy()
    out[column] = out[column].map(get_value_enum)
    return out

def add_setting_label(coverage_t: int, sample_size_source_t: int) -> str:
    return f"({coverage_t},{sample_size_source_t})"

def add_setting_column(df: pd.DataFrame) -> pd.DataFrame:
    out = df.copy()
    out["setting"] = out.apply(
        lambda row: add_setting_label(
            int(row["coverage_t"]),
            int(row["sample_size_source_t"]),
        ),
        axis=1,
    )
    return out

def blend_with_white(color: str, amount: float) -> tuple[float, float, float]:
    """
    amount = 0.0 -> white
    amount = 1.0 -> original color
    """
    r, g, b = to_rgb(color)
    return (
        1.0 - (1.0 - r) * amount,
        1.0 - (1.0 - g) * amount,
        1.0 - (1.0 - b) * amount,
    )

def make_strategy_cmap(strategy: str) -> LinearSegmentedColormap:
    base = STRATEGY_COLORS.get(strategy, "#4477AA")

    return LinearSegmentedColormap.from_list(
        f"{strategy}_heatmap",
        [
            blend_with_white(base, 0.00),  # almost white
            blend_with_white(base, 0.35),  # light
            blend_with_white(base, 0.65),  # medium
            blend_with_white(base, 1.00),  # full colour
        ],
    )

def rq1_heatMap(mat: pd.DataFrame,
    title: str,
    output_file: Path,
    colorbar_label: str,
    vmin: float | None = None,
    vmax: float | None = None,
    cmap: str = "viridis",
    ) -> None:
    fig, ax = plt.subplots(figsize=(10, 8))
    image = ax.imshow(mat.values, aspect="auto", vmin=vmin, vmax=vmax, cmap=cmap)

    ax.set_title(title)
    ax.set_xticks(range(len(mat.columns)))
    ax.set_yticks(range(len(mat.index)))
    ax.set_xticklabels(mat.columns, rotation=45, ha="right")
    ax.set_yticklabels(mat.index)
    
    cbar = fig.colorbar(image, ax=ax)
    cbar.set_label(colorbar_label)

    fig.tight_layout()
    fig.savefig(output_file, dpi=300, bbox_inches="tight")
    plt.close(fig)

def pivot_symmetric(summary_df: pd.DataFrame, value_column: str) -> pd.DataFrame:
    all_metrics = sorted(set(summary_df["metric_1"]).union(set(summary_df["metric_2"])))
    mat = pd.DataFrame(np.nan, index=all_metrics, columns=all_metrics, dtype=float)

    for _, row in summary_df.iterrows():
        metric_1 = row["metric_1"]
        metric_2 = row["metric_2"]
        value = float(row[value_column])
        mat.loc[metric_1, metric_2] = value
        mat.loc[metric_2, metric_1] = value

    for metric in all_metrics:
        mat.loc[metric, metric] = 1.0
    return mat

def plot_rq1_mean_rho_heatmap(rq1_summary: pd.DataFrame) -> None:
    mat = pivot_symmetric(rq1_summary, "mean_rho")
    rq1_heatMap(
        mat=mat,
        title="RQ1: Mean Spearman correlation between metrics",
        output_file=OUT_DIR / "rq1_mean_rho_heatmap.png",
        colorbar_label="mean rho",
        vmin=0.0,
        vmax=1.0,
        cmap="YlGnBu",
    )

def plot_rq1_setting_heatmaps(rq1: pd.DataFrame) -> None:
    settings = [
        s for s in SETTING_ORDER
        if add_setting_label(*s) in set(rq1["setting"])
    ]

    fig, axes_grid = plt.subplots(3, 3, figsize=(18, 16))
    axes = np.asarray(axes_grid).ravel()

    image = None

    for ax_idx, raw_ax in enumerate(axes):
        ax = cast(Axes, raw_ax)

        if ax_idx >= len(settings):
            ax.axis("off")
            continue

        cov_t, sample_t = settings[ax_idx]
        subset = rq1[
            (rq1["coverage_t"] == cov_t) &
            (rq1["sample_size_source_t"] == sample_t)
        ]

        all_metrics = sorted(
            set(subset["metric_1"]).union(set(subset["metric_2"]))
        )
        mat = pd.DataFrame(
            np.nan,
            index=all_metrics,
            columns=all_metrics,
            dtype=float,
        )

        for _, row in subset.iterrows():
            metric_1 = row["metric_1"]
            metric_2 = row["metric_2"]
            value = float(row["rho"])

            mat.loc[metric_1, metric_2] = value
            mat.loc[metric_2, metric_1] = value

        for metric in all_metrics:
            mat.loc[metric, metric] = 1.0

        image = ax.imshow(
            mat.values,
            aspect="auto",
            vmin=0.0,
            vmax=1.0,
            cmap="YlGnBu",
        )

        ax.set_title(f"Setting {add_setting_label(cov_t, sample_t)}")
        ax.set_xticks(range(len(all_metrics)))
        ax.set_yticks(range(len(all_metrics)))
        ax.set_xticklabels(all_metrics, rotation=90, fontsize=7)
        ax.set_yticklabels(all_metrics, fontsize=7)

    cbar_ax = fig.add_axes([0.92, 0.18, 0.015, 0.64])
    if image is not None:
        fig.colorbar(image, cax=cbar_ax, label="rho")

    fig.subplots_adjust(left=0.07, right=0.9, top=0.93, bottom=0.08, wspace=0.32, hspace=0.40)
    fig.savefig(OUT_DIR / "rq1_setting_heatmaps.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def plot_rq1_setting_heatmaps_by_strategy(rq1: pd.DataFrame) -> None:
    strategy_col = "strategy"

    if strategy_col not in rq1.columns:
        print("Skipping RQ1-by-strategy heatmaps: no 'strategy' column found.")
        return

    all_strategies = sorted(rq1[strategy_col].dropna().unique())

    for strategy in all_strategies:
        rq1_strategy = rq1[rq1[strategy_col] == strategy]
        cmap = make_strategy_cmap(str(strategy))

        settings = [
            s for s in SETTING_ORDER
            if add_setting_label(*s) in set(rq1_strategy["setting"])
        ]

        fig, axes_grid = plt.subplots(3, 3, figsize=(18, 16))
        axes = np.asarray(axes_grid).ravel()
        image = None

        for ax_idx, raw_ax in enumerate(axes):
            ax = cast(Axes, raw_ax)

            if ax_idx >= len(settings):
                ax.axis("off")
                continue

            cov_t, sample_t = settings[ax_idx]
            subset = rq1_strategy[
                (rq1_strategy["coverage_t"] == cov_t) &
                (rq1_strategy["sample_size_source_t"] == sample_t)
            ]

            all_metrics = sorted(
                set(subset["metric_1"]).union(set(subset["metric_2"]))
            )

            mat = pd.DataFrame(
                np.nan,
                index=all_metrics,
                columns=all_metrics,
                dtype=float,
            )

            for row in subset.itertuples(index=False):
                metric_1 = str(row.metric_1)
                metric_2 = str(row.metric_2)
                value = float(row.rho)

                mat.loc[metric_1, metric_2] = value
                mat.loc[metric_2, metric_1] = value

            for i in range(len(mat)):
                mat.iat[i, i] = 1.0

            image = ax.imshow(
                mat.values,
                aspect="auto",
                vmin=0.0,
                vmax=1.0,
                cmap=cmap,
            )

            ax.set_title(f"Setting {add_setting_label(cov_t, sample_t)}")
            ax.set_xticks(range(len(all_metrics)))
            ax.set_yticks(range(len(all_metrics)))
            ax.set_xticklabels(all_metrics, rotation=90, fontsize=7)
            ax.set_yticklabels(all_metrics, fontsize=7)

        fig.subplots_adjust(top=0.92, right=0.88, wspace=0.35, hspace=0.50)

        if image is not None:
            cbar_ax = fig.add_axes([0.90, 0.20, 0.02, 0.60])
            cbar = fig.colorbar(image, cax=cbar_ax)
            cbar.set_label("rho")

        fig.suptitle(f"RQ1: Metric agreement by setting ({strategy})", fontsize=16)

        safe_strategy = str(strategy).replace(" ", "_").replace("/", "_").lower()
        fig.savefig(
            OUT_DIR / f"rq1_setting_heatmaps_{safe_strategy}.png",
            dpi=300,
            bbox_inches="tight",
        )
        plt.close(fig)

def plot_rq1_mean_rho_bar(rq1_summary: pd.DataFrame)-> None:
    plot_df = rq1_summary.copy()
    plot_df["pair"] = plot_df["metric_1"] + " vs " + plot_df["metric_2"]
    plot_df = plot_df.sort_values("mean_rho", ascending=False)

    fig, ax = plt.subplots(figsize=(12, 10))
    ax.barh(plot_df["pair"], plot_df["mean_rho"])
    ax.invert_yaxis()
    ax.set_xlabel("mean_rho")
    ax.set_title("RQ1: Average agreement between metric pairs")

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq1_mean_rho_bar.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def plot_rq1_default_difference_boxplots(rq1_raw: pd.DataFrame) -> None:
    default_metric = "Default"
    all_metrics = sorted(m for m in rq1_raw["metric"].unique() if m != default_metric)
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]

    strategy_order = [
        SamplingStrategy.RANDOM.value,
        SamplingStrategy.DISTANCE.value,
        SamplingStrategy.SOLVER.value,
    ]

    offsets = {
        SamplingStrategy.RANDOM.value: -0.25,
        SamplingStrategy.DISTANCE.value: 0.0,
        SamplingStrategy.SOLVER.value: 0.25,
    }
    width = 0.22

    for other_metric in all_metrics:
        default_df = rq1_raw[rq1_raw["metric"] == default_metric].copy()
        other_df = rq1_raw[rq1_raw["metric"] == other_metric].copy()

        merge_cols = [
            "system",
            "strategy",
            "coverage_t",
            "sample_size_source_t",
        ]

        if "sample_size" in rq1_raw.columns:
            merge_cols.append("sample_size")

        paired = default_df.merge(
            other_df,
            on=merge_cols,
            suffixes=("_default", "_other"),
        )

        paired["difference"] = (
            paired["coverage_default"].astype(float)
            - paired["coverage_other"].astype(float)
        )

        paired["setting"] = paired.apply(
            lambda row: add_setting_label(
                int(row["coverage_t"]),
                int(row["sample_size_source_t"]),
            ),
            axis=1,
        )

        fig, ax = plt.subplots(figsize=(14, 6))
        base_positions = np.arange(len(ordered_settings))

        legend_handles: list[object] = []
        legend_labels: list[str] = []

        for strategy in strategy_order:
            data: list[list[float]] = []
            positions: list[float] = []

            for setting_idx, setting in enumerate(ordered_settings):
                values = paired.loc[
                    (paired["setting"] == setting) &
                    (paired["strategy"] == strategy),
                    "difference",
                ].dropna().astype(float)

                if values.empty:
                    continue

                data.append(values.tolist())
                positions.append(float(base_positions[setting_idx]) + offsets[strategy])

            if not data:
                continue

            box = ax.boxplot(
                data,
                positions=positions,
                widths=width,
                patch_artist=True,
                manage_ticks=False,
            )

            for patch in box["boxes"]:
                patch.set_alpha(0.6)

            legend_handles.append(box["boxes"][0])
            legend_labels.append(strategy)

        ax.axhline(0.0, linestyle="--", linewidth=1)

        ax.set_xticks(base_positions)
        ax.set_xticklabels(ordered_settings, rotation=45, ha="right")
        ax.set_ylabel(f"coverage difference ({default_metric} - {other_metric})")
        ax.set_title(f"RQ1: Distribution of {default_metric} - {other_metric}")

        if legend_handles:
            ax.legend(legend_handles, legend_labels, title="strategy")

        fig.tight_layout()

        safe_metric = str(other_metric).replace(" ", "_").replace("/", "_")
        fig.savefig(
            OUT_DIR / f"rq1_default_diff_boxplots_{safe_metric}.png",
            dpi=300,
            bbox_inches="tight",
        )
        plt.close(fig)



def plot_rq2_best_strategy_heatmap(rq2: pd.DataFrame) -> None:
    heat_df = rq2.pivot(index="metric", columns="setting", values="best_strategy_by_median")

    ordered_cols = [
        col for col in [add_setting_label(c, s) for c, s in SETTING_ORDER]
        if col in heat_df.columns
    ]
    heat_df = heat_df[ordered_cols]

    strategy_to_enum = {
        SamplingStrategy.RANDOM.value: 0,
        SamplingStrategy.DISTANCE.value: 1,
        SamplingStrategy.SOLVER.value: 2,
    }

    heat_num = heat_df.replace(strategy_to_enum)
    heat_num = heat_num.apply(pd.to_numeric, errors="coerce")

    cmap = ListedColormap([
        STRATEGY_COLORS[SamplingStrategy.RANDOM.value],
        STRATEGY_COLORS[SamplingStrategy.DISTANCE.value],
        STRATEGY_COLORS[SamplingStrategy.SOLVER.value],
    ])
    norm = BoundaryNorm([-0.5, 0.5, 1.5, 2.5], cmap.N)

    fig, ax = plt.subplots(figsize=(10, 6))
    image = ax.imshow(
        heat_num.to_numpy(dtype=float),
        aspect="auto",
        cmap=cmap,
        norm=norm,
    )

    ax.set_xticks(range(len(heat_df.columns)))
    ax.set_yticks(range(len(heat_df.index)))
    ax.set_xticklabels(heat_df.columns, rotation=45, ha="right")
    ax.set_yticklabels(heat_df.index)
    ax.set_title("RQ2: Best strategy by metric and setting")

    for row_idx in range(heat_df.shape[0]):
        for col_idx in range(heat_df.shape[1]):
            ax.text(
                col_idx,
                row_idx,
                str(heat_df.iloc[row_idx, col_idx]),
                ha="center",
                va="center",
                fontsize=8,
                color="white",
            )

    cbar = fig.colorbar(image, ax=ax, ticks=[0, 1, 2])
    cbar.set_ticklabels([
        SamplingStrategy.RANDOM.value,
        SamplingStrategy.DISTANCE.value,
        SamplingStrategy.SOLVER.value,
    ])

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq2_best_strategy_heatmap.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def plot_rq2_median_bars(rq2: pd.DataFrame) -> None:
    all_metrics = sorted(rq2["metric"].unique())
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]

    for metric in all_metrics:
        subset = cast(pd.DataFrame, rq2.loc[rq2["metric"] == metric].copy())
        subset["setting"] = pd.Categorical(subset["setting"], categories=ordered_settings, ordered=True)
        subset = subset.sort_values("setting")

        x = np.arange(len(subset))
        width = 0.25

        fig, ax = plt.subplots(figsize=(12, 5))
        ax.bar(
            x - width,
            subset["median_random"],
            width,
            label=SamplingStrategy.RANDOM.value,
            color=STRATEGY_COLORS[SamplingStrategy.RANDOM.value],
        )
        ax.bar(
            x,
            subset["median_distance"],
            width,
            label=SamplingStrategy.DISTANCE.value,
            color=STRATEGY_COLORS[SamplingStrategy.DISTANCE.value],
        )
        ax.bar(
            x + width,
            subset["median_solver"],
            width,
            label=SamplingStrategy.SOLVER.value,
            color=STRATEGY_COLORS[SamplingStrategy.SOLVER.value],
        )

        ax.set_xticks(x)
        ax.set_xticklabels(subset["setting"], rotation=45, ha="right")
        ax.set_ylabel("median metric value")
        ax.set_title(f"RQ2: Strategy comparison for {metric}")
        ax.legend()

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_medians_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq2_boxplots(rq2_raw: pd.DataFrame) -> None:
    all_metrics = sorted(rq2_raw["metric"].unique())
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]

    strategy_order = [
        SamplingStrategy.RANDOM.value,
        SamplingStrategy.DISTANCE.value,
        SamplingStrategy.SOLVER.value,
    ]

    offsets = {
        SamplingStrategy.RANDOM.value: -0.25,
        SamplingStrategy.DISTANCE.value: 0.0,
        SamplingStrategy.SOLVER.value: 0.25,
    }
    width = 0.22

    for metric in all_metrics:
        subset = cast(pd.DataFrame, rq2_raw.loc[rq2_raw["metric"] == metric].copy())
        subset["setting"] = pd.Categorical(
            subset["setting"],
            categories=ordered_settings,
            ordered=True,
        )
        subset = subset.sort_values(["setting", "strategy"])

        fig, ax = plt.subplots(figsize=(14, 6))
        base_positions = np.arange(len(ordered_settings))

        legend_handles: list[object] = []
        legend_labels: list[str] = []

        for strategy in strategy_order:
            data: list[list[float]] = []
            positions: list[float] = []

            for setting_idx, setting in enumerate(ordered_settings):
                values = subset.loc[
                    (subset["setting"] == setting) &
                    (subset["strategy"] == strategy),
                    "coverage",
                ].dropna().astype(float)

                if values.empty:
                    continue

                data.append(values.tolist())
                positions.append(float(base_positions[setting_idx]) + offsets[strategy])

            if not data:
                continue

            box = ax.boxplot(
                data,
                positions=positions,
                widths=width,
                patch_artist=True,
                manage_ticks=False,
            )

            for patch in box["boxes"]:
                patch.set_facecolor(STRATEGY_COLORS[strategy])
                patch.set_alpha(0.7)

            legend_handles.append(box["boxes"][0])
            legend_labels.append(strategy)

        ax.set_xticks(base_positions)
        ax.set_xticklabels(ordered_settings, rotation=45, ha="right")
        ax.set_ylabel("coverage value")
        ax.set_title(f"RQ2: Coverage distribution for {metric} by setting and strategy")

        if legend_handles:
            ax.legend(legend_handles, legend_labels, title="strategy")

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_boxplots_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq2_strategy_wins(rq2_summary: pd.DataFrame) -> None:
    plot_df = rq2_summary.sort_values("metric")
    x = np.arange(len(plot_df))

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(
        x,
        plot_df["random_wins"],
        label=SamplingStrategy.RANDOM.value,
        color=STRATEGY_COLORS[SamplingStrategy.RANDOM.value],
    )
    ax.bar(
        x,
        plot_df["distance_wins"],
        bottom=plot_df["random_wins"],
        label=SamplingStrategy.DISTANCE.value,
        color=STRATEGY_COLORS[SamplingStrategy.DISTANCE.value],
    )
    ax.bar(
        x,
        plot_df["solver_wins"],
        bottom=plot_df["random_wins"] + plot_df["distance_wins"],
        label=SamplingStrategy.SOLVER.value,
        color=STRATEGY_COLORS[SamplingStrategy.SOLVER.value],
    )

    ax.set_xticks(x)
    ax.set_xticklabels(plot_df["metric"], rotation=45, ha="right")
    ax.set_ylabel("number of settings won")
    ax.set_title("RQ2 summary: strategy wins per metric")
    ax.legend()

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq2_strategy_wins.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def get_strategy_ranks(row: pd.Series) -> dict[str, int]:
    scores = [
        (SamplingStrategy.RANDOM.value, float(row["median_random"])),
        (SamplingStrategy.DISTANCE.value, float(row["median_distance"])),
        (SamplingStrategy.SOLVER.value, float(row["median_solver"])),
    ]

    scores.sort(key=lambda item: item[1], reverse=True)

    ranks: dict[str, int] = {}
    current_rank = 1
    previous_score: float | None = None

    for position, (strategy, score) in enumerate(scores, start=1):
        if previous_score is not None and score < previous_score:
            current_rank = position
        ranks[strategy] = current_rank
        previous_score = score

    return ranks

def plot_rq2_full_ordering_bars(rq2: pd.DataFrame) -> None:
    all_metrics = sorted(rq2["metric"].unique())

    for metric in all_metrics:
        subset = rq2[rq2["metric"] == metric].copy()

        rows = []
        for row in subset.itertuples(index=False):
            rows.append({
                "label": f"random {row.setting}",
                "value": float(row.median_random),
                "strategy": SamplingStrategy.RANDOM.value,
            })
            rows.append({
                "label": f"distance {row.setting}",
                "value": float(row.median_distance),
                "strategy": SamplingStrategy.DISTANCE.value,
            })
            rows.append({
                "label": f"solver {row.setting}",
                "value": float(row.median_solver),
                "strategy": SamplingStrategy.SOLVER.value,
            })

        plot_df = pd.DataFrame(rows).sort_values("value", ascending=False)
        colors = [STRATEGY_COLORS[strategy] for strategy in plot_df["strategy"]]

        fig, ax = plt.subplots(figsize=(12, 10))
        ax.barh(plot_df["label"], plot_df["value"], color=colors)
        ax.invert_yaxis()
        ax.set_xlabel("median metric value")
        ax.set_title(f"RQ2: Full ordering for {metric}")

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_fullordering{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)


def plot_rq3_trend_lines(rq3: pd.DataFrame) -> None:
    all_metrics = sorted(rq3["metric"].unique())
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]

    for metric in all_metrics:
        subset = cast(pd.DataFrame, rq3.loc[rq3["metric"] == metric].copy())
        subset["setting"] = pd.Categorical(subset["setting"], categories=ordered_settings, ordered=True)
        subset = subset.sort_values("setting")

        fig, ax = plt.subplots(figsize=(12, 5))

        for _, row in subset.iterrows():
            medians = [
                float(row["median_weak"]),
                float(row["median_moderate"]),
                float(row["median_strong"]),
                float(row["median_verystrong"])
            ]
            ax.plot(CONSTRAINT_ORDER, medians, marker="o", label=row["setting"])

        ax.set_ylabel("median metric value")
        ax.set_title(f"RQ3: Constraint-strength trends for {metric}")
        ax.legend(title="setting", bbox_to_anchor=(1.02, 1), loc="upper left")

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq3_trends_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq3_weak_minus_verystrong_heatmap(rq3: pd.DataFrame) -> None:
    heat = rq3.pivot(index="metric", columns="setting", values="weak_minus_verystrong")
    ordered_cols = [col for col in [add_setting_label(c, s) for c, s in SETTING_ORDER] if col in heat.columns]
    heat = heat[ordered_cols]

    fig, ax = plt.subplots(figsize=(10, 6))
    image = ax.imshow(heat.values, aspect="auto", cmap="coolwarm")

    ax.set_xticks(range(len(heat.columns)))
    ax.set_yticks(range(len(heat.index)))
    ax.set_xticklabels(heat.columns, rotation=45, ha="right")
    ax.set_yticklabels(heat.index)
    ax.set_title("RQ3: Weak minus Very Strong median gap")

    for row_idx in range(heat.shape[0]):
        for col_idx in range(heat.shape[1]):
            value = heat.iloc[row_idx, col_idx]
            if pd.notna(value):
                ax.text(col_idx, row_idx, f"{value:.2f}", ha="center", va="center", fontsize=8)

    cbar = fig.colorbar(image, ax=ax)
    cbar.set_label("median_weak - median_verystrong")

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq3_weak_minus_verystrong_heatmap.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def plot_rq3_trend_direction_summary(rq3_summary: pd.DataFrame) -> None:
    plot_df = rq3_summary.sort_values("metric")
    x  = np.arange(len(plot_df))

    fig, ax = plt.subplots(figsize=(10, 5))
    ax.bar(x, plot_df["n_non_monotonic"], label="non-monotonic")
    ax.bar(
        x,
        plot_df["n_increasing"],
        bottom=plot_df["n_non_monotonic"],
        label="increasing",
    )
    ax.bar(
        x,
        plot_df["n_decreasing"],
        bottom=plot_df["n_non_monotonic"] + plot_df["n_increasing"],
        label="decreasing",
    )

    ax.set_xticks(x)
    ax.set_xticklabels(plot_df["metric"], rotation=45, ha="right")
    ax.set_ylabel("number of settings")
    ax.set_title("RQ3 summary: trend types by metric")
    ax.legend()

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq3_trend_direction_summary.png", dpi=300, bbox_inches="tight")
    plt.close(fig)

def plot_rq3_system_counts_by_constraint_level(rq3_raw: pd.DataFrame) -> None:
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]
    strength_order = [
        Strength.WEAK.value,
        Strength.MODERATE.value,
        Strength.STRONG.value,
        Strength.VERYSTRONG.value,
    ]

    counts = (
        rq3_raw.groupby(["setting", "constraint_level"])["system"]
        .nunique()
        .reset_index(name="n_systems")
    )

    print(sorted(rq3_raw["setting"].dropna().unique()))
    print(rq3_raw.groupby("setting")["system"].nunique())

    counts["setting"] = pd.Categorical(
        counts["setting"],
        categories=ordered_settings,
        ordered=True,
    )
    counts = counts.sort_values(["setting", "constraint_level"])

    x = np.arange(len(ordered_settings))
    width = 0.2
    offsets = [-0.3, -0.1, 0.1, 0.3]

    fig, ax = plt.subplots(figsize=(12, 6))

    for idx, strength in enumerate(strength_order):
        subset = counts[counts["constraint_level"] == strength]
        y: list[int] = []

        for setting in ordered_settings:
            match = subset[subset["setting"] == setting]
            if match.empty:
                y.append(0)
            else:
                y.append(int(match["n_systems"].iloc[0]))

        ax.bar(
            x + offsets[idx],
            y,
            width,
            label=strength,
            color=CONSTRAINT_COLORS[strength],
        )

    ax.set_xticks(x)
    ax.set_xticklabels(ordered_settings, rotation=45, ha="right")
    ax.set_ylabel("number of systems")
    ax.set_title("RQ3: Number of systems per constraint level and setting")
    ax.legend(title="constraint level")

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq3_system_counts_by_constraint_level.png", dpi=300, bbox_inches="tight")
    plt.close(fig)


def plot_rq3_boxplots(rq3_raw: pd.DataFrame) -> None:
    all_metrics = sorted(rq3_raw["metric"].unique())
    ordered_settings = [add_setting_label(c, s) for c, s in SETTING_ORDER]

    strength_order = [
        Strength.WEAK.value,
        Strength.MODERATE.value,
        Strength.STRONG.value,
        Strength.VERYSTRONG.value,
    ]

    offsets = {
        Strength.WEAK.value: -0.30,
        Strength.MODERATE.value: -0.10,
        Strength.STRONG.value: 0.10,
        Strength.VERYSTRONG.value: 0.30,
    }
    width = 0.16

    for metric in all_metrics:
        subset = cast(pd.DataFrame, rq3_raw.loc[rq3_raw["metric"] == metric].copy())
        subset["setting"] = pd.Categorical(
            subset["setting"],
            categories=ordered_settings,
            ordered=True,
        )
        subset = subset.sort_values(["setting", "constraint_level"])

        fig, ax = plt.subplots(figsize=(14, 6))
        base_positions = np.arange(len(ordered_settings))

        legend_handles: list[object] = []
        legend_labels: list[str] = []

        for strength in strength_order:
            data: list[list[float]] = []
            positions: list[float] = []

            for setting_idx, setting in enumerate(ordered_settings):
                values = subset.loc[
                    (subset["setting"] == setting) &
                    (subset["constraint_level"] == strength),
                    "coverage",
                ].dropna().astype(float)

                if values.empty:
                    continue

                data.append(values.tolist())
                positions.append(float(base_positions[setting_idx]) + offsets[strength])

            if not data:
                continue

            box = ax.boxplot(
                data,
                positions=positions,
                widths=width,
                patch_artist=True,
                manage_ticks=False,
            )

            for patch in box["boxes"]:
                patch.set_facecolor(CONSTRAINT_COLORS[strength])
                patch.set_alpha(0.7)

            legend_handles.append(box["boxes"][0])
            legend_labels.append(strength)

        ax.set_xticks(base_positions)
        ax.set_xticklabels(ordered_settings, rotation=45, ha="right")
        ax.set_ylabel("coverage value")
        ax.set_title(f"RQ3: Coverage distribution for {metric} by setting and constraint strength")

        if legend_handles:
            ax.legend(legend_handles, legend_labels, title="constraint strength")

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq3_boxplots_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def get_data() -> dict[str, pd.DataFrame]:
    rq1_results = pd.read_csv(RQ1_RESULTS)
    rq1_summary = pd.read_csv(RQ1_SUMMARY)
    rq1_raw = pd.read_csv(RQ1_RAW)

    rq2_results = pd.read_csv(RQ2_RESULTS)
    rq2_summary = pd.read_csv(RQ2_SUMMARY)
    rq2_raw = pd.read_csv(RQ2_RAW)

    rq3_results = pd.read_csv(RQ3_RESULTS)
    rq3_summary = pd.read_csv(RQ3_SUMMARY)
    rq3_raw = pd.read_csv(RQ3_RAW)

    rq1_results = get_value_enum_column(rq1_results, "rho_strength")
    rq2_results = get_value_enum_column(rq2_results, "effect_strength")
    rq3_results = get_value_enum_column(rq3_results, "effect_strength")

    rq1_results = add_setting_column(rq1_results)
    rq2_results = add_setting_column(rq2_results)
    rq3_results = add_setting_column(rq3_results)

    return {
        "rq1": rq1_results,
        "rq1_summary": rq1_summary,
        "rq1_raw": rq1_raw,
        "rq2": rq2_results,
        "rq2_summary": rq2_summary,
        "rq2_raw": rq2_raw,
        "rq3": rq3_results,
        "rq3_summary": rq3_summary,
        "rq3_raw": rq3_raw,
    }

def main() -> None:
    data = get_data()

    plot_rq1_mean_rho_heatmap(data["rq1_summary"])
    plot_rq1_setting_heatmaps(data["rq1"])
    plot_rq1_mean_rho_bar(data["rq1_summary"])
    plot_rq1_setting_heatmaps_by_strategy(data["rq1"])
    plot_rq1_default_difference_boxplots(data["rq1_raw"])

    plot_rq2_best_strategy_heatmap(data["rq2"])
    plot_rq2_full_ordering_bars(data["rq2"])
    plot_rq2_median_bars(data["rq2"])
    plot_rq2_strategy_wins(data["rq2_summary"])
    plot_rq2_boxplots(data["rq2_raw"])

    plot_rq3_trend_lines(data["rq3"])
    plot_rq3_weak_minus_verystrong_heatmap(data["rq3"])
    plot_rq3_trend_direction_summary(data["rq3_summary"])
    plot_rq3_boxplots(data["rq3_raw"])
    plot_rq3_system_counts_by_constraint_level(data["rq3_raw"])

    print(f"Plots written to: {OUT_DIR.resolve()}")


if __name__ == "__main__":
    main()