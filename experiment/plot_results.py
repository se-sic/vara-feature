#!/usr/bin/env python3

from pathlib import Path
from experiment_config import Strength, SamplingStrategy
from typing import cast
from matplotlib.axes import Axes
from matplotlib.patches import Rectangle
from matplotlib.colors import LinearSegmentedColormap, ListedColormap, BoundaryNorm, to_rgb
from matplotlib.patches import Patch

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import numpy.typing as npt



plt.rcParams.update({
    "font.size": 22,
    "axes.titlesize": 24,
    "axes.labelsize": 24,
    "xtick.labelsize": 20,
    "ytick.labelsize": 20,
    "legend.fontsize": 18,
    "legend.title_fontsize": 19,
    "axes.linewidth": 1.2,
    "lines.linewidth": 2.0,
    "patch.linewidth": 1.4,
    "xtick.major.width": 1.2,
    "ytick.major.width": 1.2,
    "savefig.dpi": 300,
    "savefig.bbox": "tight",
    "figure.constrained_layout.use": False,
})

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
    SamplingStrategy.RANDOM.value: "#4477AA",   
    SamplingStrategy.DISTANCE.value: "#228833", 
    SamplingStrategy.SOLVER.value: "#AA3377",   
}

CONSTRAINT_COLORS = {
    Strength.WEAK.value: "#D1C1E1",
    Strength.MODERATE.value: "#B58FC2",
    Strength.STRONG.value: "#9B62A7",
    Strength.VERYSTRONG.value: "#6F4C9B",
}


def export_standalone_legend(
    labels: list[str],
    colors: list[str],
    output_file: Path,
    title: str | None = None,
    ncol: int | None = None,
    fontsize: int = 20,
    title_fontsize: int = 22,
) -> None:
    """Render a legend on its own and save it as a standalone PNG.

    Used so the individual data figures can omit their legends (they are
    repeated across many subfigures) and the legend can be placed once in
    LaTeX instead.
    """
    handles = [
        Patch(facecolor=color, edgecolor="black", alpha=0.7, label=label)
        for label, color in zip(labels, colors)
    ]

    if ncol is None:
        ncol = len(labels)

    fig = plt.figure(figsize=(0.1, 0.1))
    legend = fig.legend(
        handles=handles,
        loc="center",
        ncol=ncol,
        title=title,
        frameon=True,
        fontsize=fontsize,
        title_fontsize=title_fontsize,
    )
    fig.canvas.draw()
    bbox = legend.get_window_extent().transformed(fig.dpi_scale_trans.inverted())
    fig.savefig(output_file, dpi=300, bbox_inches=bbox, pad_inches=0.05)
    plt.close(fig)


def export_all_legends(rq3_raw: pd.DataFrame | None = None) -> None:
    """Write the three standalone legend PNGs used across the figures."""
    export_standalone_legend(
        labels=[
            SamplingStrategy.RANDOM.value,
            SamplingStrategy.DISTANCE.value,
            SamplingStrategy.SOLVER.value,
        ],
        colors=[
            STRATEGY_COLORS[SamplingStrategy.RANDOM.value],
            STRATEGY_COLORS[SamplingStrategy.DISTANCE.value],
            STRATEGY_COLORS[SamplingStrategy.SOLVER.value],
        ],
        output_file=OUT_DIR / "legend_strategy.png",
        title="strategy",
    )

    export_standalone_legend(
        labels=CONSTRAINT_ORDER,
        colors=[CONSTRAINT_COLORS[s] for s in CONSTRAINT_ORDER],
        output_file=OUT_DIR / "legend_constraint.png",
        title="constraint strength",
    )

    if rq3_raw is not None:
        metric_order = sorted(rq3_raw["metric"].dropna().unique())
        palette = [
            "#332288", "#88CCEE", "#44AA99", "#117733",
            "#999933", "#DDCC77", "#CC6677", "#AA4499",
        ]
        export_standalone_legend(
            labels=[str(m) for m in metric_order],
            colors=palette[: len(metric_order)],
            output_file=OUT_DIR / "legend_metric.png",
            title="metric",
            ncol=min(4, len(metric_order)),
        )


TITLE_FONTSIZE = 24
AXIS_LABEL_FONTSIZE = 24
TICK_FONTSIZE = 20
LEGEND_FONTSIZE = 18
LEGEND_TITLE_FONTSIZE = 19
ANNOTATION_FONTSIZE = 16

def style_heatmap_axes(
    ax: Axes,
    row_idx: int,
    col_idx: int,
    n_rows: int,
    n_cols: int,
    xlabels: list[str],
    ylabels: list[str],
    x_rotation: int = 90,
    x_fontsize: int = TICK_FONTSIZE,
    y_fontsize: int = TICK_FONTSIZE,
) -> None:
    show_x = (row_idx == n_rows - 1)
    show_y = (col_idx == 0)

    ax.set_xticks(range(len(xlabels)))
    ax.set_yticks(range(len(ylabels)))

    if show_x:
        ax.set_xticklabels(
            xlabels,
            rotation=x_rotation,
            ha="right",
            fontsize=x_fontsize,
        )
        ax.tick_params(axis="x", bottom=True, labelbottom=True)
    else:
        ax.set_xticklabels([])
        ax.tick_params(axis="x", bottom=False, labelbottom=False)

    if show_y:
        ax.set_yticklabels(ylabels, fontsize=y_fontsize)
        ax.tick_params(axis="y", left=True, labelleft=True)
    else:
        ax.set_yticklabels([])
        ax.tick_params(axis="y", left=False, labelleft=False)

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
    return f"$t_c={coverage_t},\\; t_s={sample_size_source_t}$"

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

    r, g, b = to_rgb(color)
    return (
        1.0 - (1.0 - r) * amount,
        1.0 - (1.0 - g) * amount,
        1.0 - (1.0 - b) * amount,
    )

def safe_name(value: object) -> str:
    return str(value).replace(" ", "_").replace("/", "_").lower()

def make_strategy_cmap(strategy: str) -> LinearSegmentedColormap:
    base = STRATEGY_COLORS.get(strategy, "#4477AA")

    return LinearSegmentedColormap.from_list(
        f"{strategy}_heatmap",
        [
            blend_with_white(base, 0.00), 
            blend_with_white(base, 0.35), 
            blend_with_white(base, 0.65), 
            blend_with_white(base, 1.00),  
        ],
    )

def rq1_heatMap(
    mat: pd.DataFrame,
    title: str,
    output_file: Path,
    colorbar_label: str,
    vmin: float | None = None,
    vmax: float | None = None,
    cmap: str = "viridis",
) -> None:
    fig, ax = plt.subplots(figsize=(10, 8))
    image = ax.imshow(mat.values, aspect="auto", vmin=vmin, vmax=vmax, cmap=cmap)

    #ax.set_title(title, fontsize=TITLE_FONTSIZE)
    ax.set_xticks(range(len(mat.columns)))
    ax.set_yticks(range(len(mat.index)))
    ax.set_xticklabels(mat.columns, rotation=45, ha="right", fontsize=TICK_FONTSIZE)
    ax.set_yticklabels(mat.index, fontsize=TICK_FONTSIZE)

    cbar = fig.colorbar(image, ax=ax)
    cbar.ax.tick_params(labelsize=TICK_FONTSIZE)
    cbar.set_label(colorbar_label, fontsize=AXIS_LABEL_FONTSIZE)

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

    fig, axes_grid = plt.subplots(3, 3, figsize=(15, 14))
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

        pair_mean = (
            subset.groupby(["metric_1", "metric_2"])["rho"]
            .mean()
            .resetindex()
        )

        for _, row in pair_mean.iterrows():
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

        ax.set_title(
            f"$t_c={cov_t},\\; t_s={sample_t}$",
            fontsize=22,
        )

        row_idx, col_idx = divmod(ax_idx, 3)
        style_heatmap_axes(
            ax=ax,
            row_idx=row_idx,
            col_idx=col_idx,
            n_rows=3,
            n_cols=3,
            xlabels=all_metrics,
            ylabels=all_metrics,
            x_rotation=45,
            x_fontsize=18,
            y_fontsize=18,
        )

    cbar_ax = fig.add_axes([0.92, 0.18, 0.015, 0.64])
    if image is not None:
        cbar = fig.colorbar(image, cax=cbar_ax)
        cbar.set_label("rho", fontsize=20)
        cbar.ax.tick_params(labelsize=16)

    fig.subplots_adjust(
        left=0.06,
        right=0.91,
        top=0.95,
        bottom=0.07,
        wspace=0.30,
        hspace=0.28,
    )
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

        fig, axes_grid = plt.subplots(3, 3, figsize=(15, 14))
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

            ax.set_title(
                f"$t_c={cov_t},\\; t_s={sample_t}$",
                fontsize=18,
            )

            row_idx, col_idx = divmod(ax_idx, 3)
            style_heatmap_axes(
                ax=ax,
                row_idx=row_idx,
                col_idx=col_idx,
                n_rows=3,
                n_cols=3,
                xlabels=all_metrics,
                ylabels=all_metrics,
                x_rotation=45,
                x_fontsize=14,
                y_fontsize=14,
            )

        fig.subplots_adjust(
            left=0.06,
            right=0.90,
            top=0.95,
            bottom=0.07,
            wspace=0.18,
            hspace=0.22,
        )

        if image is not None:
            cbar_ax = fig.add_axes([0.91, 0.16, 0.015, 0.68])
            cbar = fig.colorbar(image, cax=cbar_ax)
            cbar.set_label("rho", fontsize=20)
            cbar.ax.tick_params(labelsize=16)

        # fig.suptitle(
        #     f"RQ1: Metric agreement by setting ({strategy})",
        #     fontsize=TITLE_FONTSIZE + 2,
        # )

        fig.savefig(
            OUT_DIR / f"rq1_setting_heatmaps_{safe_name(strategy)}.png",
            dpi=300,
            bbox_inches="tight", pad_inches=0.02,
        )
        plt.close(fig)

def plot_rq1_mean_rho_bar(rq1_summary: pd.DataFrame)-> None:
    plot_df = rq1_summary.copy()
    plot_df["pair"] = plot_df["metric_1"] + " vs " + plot_df["metric_2"]
    plot_df = plot_df.sort_values("mean_rho", ascending=False)

    fig, ax = plt.subplots(figsize=(12, 10))
    ax.barh(plot_df["pair"], plot_df["mean_rho"])
    ax.invert_yaxis()
    ax.set_xlabel("mean_rho", fontsize=AXIS_LABEL_FONTSIZE)
    # ax.set_title("RQ1: Average agreement between metric pairs", fontsize=TITLE_FONTSIZE)

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

        fig, ax = plt.subplots(figsize=(11, 8))
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
                patch.set_facecolor(STRATEGY_COLORS[strategy])
                patch.set_alpha(0.6)

            legend_handles.append(box["boxes"][0])
            legend_labels.append(strategy)

        ax.axhline(0.0, linestyle="--", linewidth=1)

        ax.set_xticks(base_positions)
        ax.set_xticklabels(ordered_settings, rotation=45, ha="right")
        ax.tick_params(axis="both", labelsize=TICK_FONTSIZE)
        ax.set_ylabel(f"coverage difference", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ1: Distribution of {default_metric} - {other_metric}", fontsize=TITLE_FONTSIZE)

        if legend_handles:
            ax.legend(
                legend_handles,
                legend_labels,
                title="strategy",
                fontsize=LEGEND_FONTSIZE,
                title_fontsize=LEGEND_TITLE_FONTSIZE,
            )

        fig.tight_layout()

        fig.savefig(
            OUT_DIR / f"rq1_default_diff_boxplots_{safe_name(other_metric)}.png",
            dpi=300,
            bbox_inches="tight",
        )
        plt.close(fig)

def plot_rq1_setting_heatmaps_by_strategy_split(rq1: pd.DataFrame) -> None:
    strategy_col = "strategy"

    if strategy_col not in rq1.columns:
        print("Skipping split RQ1-by-strategy heatmaps: no 'strategy' column found.")
        return

    all_strategies = sorted(rq1[strategy_col].dropna().unique())
    coverage_t_order = [1, 2, 3]
    source_t_order = [1, 2, 3]

    for strategy in all_strategies:
        rq1_strategy = rq1[rq1[strategy_col] == strategy]
        cmap = make_strategy_cmap(str(strategy))

        for source_t in source_t_order:
            fig, axes_grid = plt.subplots(
                1, 3,
                figsize=(15.5, 5.6),
                gridspec_kw={"wspace": 0.18},
            )
            axes = np.asarray(axes_grid).ravel()
            image = None

            for ax_idx, coverage_t in enumerate(coverage_t_order):
                ax = cast(Axes, axes[ax_idx])

                subset = rq1_strategy[
                    (rq1_strategy["coverage_t"] == coverage_t) &
                    (rq1_strategy["sample_size_source_t"] == source_t)
                ]

                if subset.empty:
                    ax.axis("off")
                    continue

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

                ax.set_title(f"coverage $t_c={coverage_t}$", fontsize=18)

                row_idx, col_idx = divmod(ax_idx, 3)
                style_heatmap_axes(
                    ax=ax,
                    row_idx=row_idx,
                    col_idx=col_idx,
                    n_rows=1,
                    n_cols=3,
                    xlabels=all_metrics,
                    ylabels=all_metrics,
                    x_rotation=45,
                    x_fontsize=15,
                    y_fontsize=15,
                )

            fig.subplots_adjust(
                left=0.08,
                right=0.93,
                top=0.88,
                bottom=0.20,
                wspace=0.18,
            )

            if image is not None:
                cbar = fig.colorbar(image, ax=axes, fraction=0.025, pad=0.02)
                cbar.set_label("rho", fontsize=16)
                cbar.ax.tick_params(labelsize=13)

            fig.savefig(
                OUT_DIR / f"rq1_setting_heatmaps_{safe_name(strategy)}_source_{source_t}.png",
                dpi=300,
                bbox_inches="tight",
                pad_inches=0.02,
            )
            plt.close(fig)

def plot_rq1_default_difference_boxplots_split(rq1_raw: pd.DataFrame) -> None:
    default_metric = "Default"
    all_metrics = sorted(m for m in rq1_raw["metric"].unique() if m != default_metric)

    strategy_order = [
        SamplingStrategy.RANDOM.value,
        SamplingStrategy.DISTANCE.value,
        SamplingStrategy.SOLVER.value,
    ]
    coverage_t_order = [1, 2, 3]
    source_t_order = [1, 2, 3]

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

        for source_t in source_t_order:
            subset = paired[paired["sample_size_source_t"] == source_t].copy()
            if subset.empty:
                continue

            fig, ax = plt.subplots(figsize=(11, 8))
            base_positions = np.arange(len(coverage_t_order))

            legend_handles: list[object] = []
            legend_labels: list[str] = []

            for strategy in strategy_order:
                data: list[list[float]] = []
                positions: list[float] = []

                for idx, coverage_t in enumerate(coverage_t_order):
                    values = subset.loc[
                        (subset["coverage_t"] == coverage_t) &
                        (subset["strategy"] == strategy),
                        "difference",
                    ].dropna().astype(float)

                    if values.empty:
                        continue

                    data.append(values.tolist())
                    positions.append(float(base_positions[idx]) + offsets[strategy])

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
                    patch.set_alpha(0.6)

                legend_handles.append(box["boxes"][0])
                legend_labels.append(strategy)

            ax.axhline(0.0, linestyle="--", linewidth=1)

            ax.set_xticks(base_positions)
            ax.set_xticklabels([str(t) for t in coverage_t_order])
            ax.tick_params(axis="both", labelsize=TICK_FONTSIZE)
            ax.set_xlabel("coverage $t_c$", fontsize=AXIS_LABEL_FONTSIZE)
            ax.set_ylabel(f"coverage difference", fontsize=AXIS_LABEL_FONTSIZE)
            # ax.set_title(
            #     f"RQ1: Distribution of {default_metric} - {other_metric} "
            #     f"(sample size source t={source_t})",
            #     fontsize=TITLE_FONTSIZE,
            # )

            if legend_handles:
                ax.legend(
                    legend_handles,
                    legend_labels,
                    title="strategy",
                    fontsize=LEGEND_FONTSIZE,
                    title_fontsize=LEGEND_TITLE_FONTSIZE,
                )

            fig.tight_layout()
            fig.savefig(
                OUT_DIR / f"rq1_default_diff_boxplots_{safe_name(other_metric)}_source_{source_t}.png",
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

    fig, ax = plt.subplots(figsize=(13, 7.5))
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
    ax.tick_params(axis="x", labelrotation=45, labelsize=12)
    ax.tick_params(axis="y", labelsize=12)
    #ax.set_title("RQ2: Best strategy by metric and setting", fontsize=TITLE_FONTSIZE)

    for row_idx in range(heat_df.shape[0]):
        for col_idx in range(heat_df.shape[1]):
            ax.text(
                col_idx,
                row_idx,
                str(heat_df.iloc[row_idx, col_idx]),
                ha="center",
                va="center",
                fontsize=9,
                color="white",
            )

    cbar = fig.colorbar(image, ax=ax, ticks=[0, 1, 2])
    cbar.set_ticklabels([
        SamplingStrategy.RANDOM.value,
        SamplingStrategy.DISTANCE.value,
        SamplingStrategy.SOLVER.value,
    ])

    fig.tight_layout()
    fig.savefig(OUT_DIR / "rq2_best_strategy_heatmap.png", dpi=300, bbox_inches="tight", pad_inches=0.02,)
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

        fig, ax = plt.subplots(figsize=(11, 8))
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
        ax.tick_params(axis="both", labelsize=TICK_FONTSIZE)
        ax.set_ylabel("median metric value", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ2: Strategy comparison for {metric}", fontsize=TITLE_FONTSIZE)
        ax.legend(fontsize=LEGEND_FONTSIZE, title_fontsize=LEGEND_TITLE_FONTSIZE)

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_medians_{safe_name(metric)}.png", dpi=300, bbox_inches="tight", pad_inches=0.02)
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

        fig, ax = plt.subplots(figsize=(11, 8))
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
        ax.tick_params(axis="both", labelsize=TICK_FONTSIZE)
        ax.set_ylabel("coverage value", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ2: Coverage distribution for {metric} by setting and strategy", fontsize=TITLE_FONTSIZE)

        if legend_handles:
            ax.legend(
                legend_handles,
                legend_labels,
                title="strategy",
                fontsize=LEGEND_FONTSIZE,
                title_fontsize=LEGEND_TITLE_FONTSIZE,
            )

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_boxplots_{safe_name(metric)}.png", dpi=300, bbox_inches="tight", pad_inches=0.02)
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
    ax.set_ylabel("number of settings won", fontsize=AXIS_LABEL_FONTSIZE)
    #ax.set_title("RQ2 summary: strategy wins per metric", fontsize=TITLE_FONTSIZE)
    ax.legend(fontsize=LEGEND_FONTSIZE)

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
        ax.set_xlabel("median metric value", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ2: Full ordering for {metric}", fontsize=TITLE_FONTSIZE)

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq2_fullordering{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq2_median_bars_split(rq2: pd.DataFrame) -> None:
    all_metrics = sorted(rq2["metric"].unique())
    coverage_t_order = [1, 2, 3]
    source_t_order = [1, 2, 3]

    for metric in all_metrics:
        metric_df = cast(pd.DataFrame, rq2.loc[rq2["metric"] == metric].copy())

        for source_t in source_t_order:
            subset = cast(
                pd.DataFrame,
                metric_df.loc[metric_df["sample_size_source_t"] == source_t].copy(),
            )

            if subset.empty:
                continue

            subset["coverage_t"] = pd.Categorical(
                subset["coverage_t"],
                categories=coverage_t_order,
                ordered=True,
            )
            subset = subset.sort_values("coverage_t")

            x = np.arange(len(subset))
            width = 0.25

            fig, ax = plt.subplots(figsize=(11, 8))
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
            ax.set_xticklabels([str(t) for t in subset["coverage_t"]])
            ax.tick_params(axis="both", labelsize=TICK_FONTSIZE)
            ax.set_xlabel("coverage $t_c$", fontsize=AXIS_LABEL_FONTSIZE)
            ax.set_ylabel("median metric value", fontsize=AXIS_LABEL_FONTSIZE)
            # ax.set_title(
            #     f"RQ2: Strategy comparison for {metric} (sample size source t={source_t})",
            #     fontsize=TITLE_FONTSIZE,
            # )
            ax.legend(fontsize=LEGEND_FONTSIZE, title_fontsize=LEGEND_TITLE_FONTSIZE)

            fig.tight_layout()
            fig.savefig(
                OUT_DIR / f"rq2_medians_{safe_name(metric)}_source_{source_t}.png",
                dpi=300,
                bbox_inches="tight", pad_inches=0.02
            )
            plt.close(fig)

def plot_rq2_boxplots_split(rq2_raw: pd.DataFrame) -> None:
    all_metrics = sorted(rq2_raw["metric"].unique())
    coverage_t_order = [1, 2, 3]
    source_t_order = [1, 2, 3]

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
        metric_df = cast(pd.DataFrame, rq2_raw.loc[rq2_raw["metric"] == metric].copy())

        for source_t in source_t_order:
            subset = cast(
                pd.DataFrame,
                metric_df.loc[metric_df["sample_size_source_t"] == source_t].copy(),
            )

            if subset.empty:
                continue

            subset["coverage_t"] = pd.Categorical(
                subset["coverage_t"],
                categories=coverage_t_order,
                ordered=True,
            )
            subset = subset.sort_values(["coverage_t", "strategy"])

            fig, ax = plt.subplots(figsize=(11, 8))
            base_positions = np.arange(len(coverage_t_order))

            legend_handles: list[object] = []
            legend_labels: list[str] = []

            for strategy in strategy_order:
                data: list[list[float]] = []
                positions: list[float] = []

                for idx, coverage_t in enumerate(coverage_t_order):
                    values = subset.loc[
                        (subset["coverage_t"] == coverage_t) &
                        (subset["strategy"] == strategy),
                        "coverage",
                    ].dropna().astype(float)

                    if values.empty:
                        continue

                    data.append(values.tolist())
                    positions.append(float(base_positions[idx]) + offsets[strategy])

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
            ax.set_xticklabels([str(t) for t in coverage_t_order])
            ax.set_xlabel("coverage $t_c$", fontsize=AXIS_LABEL_FONTSIZE)
            ax.set_ylabel("coverage value", fontsize=AXIS_LABEL_FONTSIZE)
            # ax.set_title(
            #     f"RQ2: Coverage distribution for {metric} "
            #     f"(sample size source t={source_t})", fontsize=TITLE_FONTSIZE
            # )

            if legend_handles:
                ax.legend(legend_handles, legend_labels, title="strategy", fontsize=LEGEND_FONTSIZE,
                title_fontsize=LEGEND_TITLE_FONTSIZE,)

            fig.tight_layout()
            fig.savefig(
                OUT_DIR / f"rq2_boxplots_{safe_name(metric)}_source_{source_t}.png",
                dpi=300,
                bbox_inches="tight", pad_inches=0.02
            )
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

        ax.set_ylabel("median metric value", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ3: Constraint-strength trends for {metric}", fontsize=TITLE_FONTSIZE)
        ax.legend(title="setting", bbox_to_anchor=(1.02, 1), loc="upper left", fontsize=LEGEND_FONTSIZE,
    title_fontsize=LEGEND_TITLE_FONTSIZE,)

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq3_trends_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq3_weak_minus_verystrong_heatmap(rq3: pd.DataFrame) -> None:
    heat = rq3.pivot(index="metric", columns="setting", values="weak_minus_verystrong")
    ordered_cols = [col for col in [add_setting_label(c, s) for c, s in SETTING_ORDER] if col in heat.columns]
    heat = heat[ordered_cols]

    fig, ax = plt.subplots(figsize=(13, 7.5))
    image = ax.imshow(heat.values, aspect="auto", cmap="coolwarm")

    ax.set_xticks(range(len(heat.columns)))
    ax.set_yticks(range(len(heat.index)))
    ax.set_xticklabels(heat.columns, rotation=45, ha="right")
    ax.set_yticklabels(heat.index)
    ax.tick_params(axis="x", labelrotation=45, labelsize=12)
    ax.tick_params(axis="y", labelsize=12)
    #ax.set_title("RQ3: Weak minus Very Strong median gap", fontsize=TITLE_FONTSIZE)

    for row_idx in range(heat.shape[0]):
        for col_idx in range(heat.shape[1]):
            value = heat.iloc[row_idx, col_idx]
            if pd.notna(value):
                ax.text(col_idx, row_idx, f"{value:.2f}", ha="center", va="center", fontsize=9)

    cbar = fig.colorbar(image, ax=ax)
    cbar.ax.tick_params(labelsize=TICK_FONTSIZE)
    cbar.set_label("median_weak - median_verystrong", fontsize=AXIS_LABEL_FONTSIZE)

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
    ax.set_ylabel("number of settings", fontsize=AXIS_LABEL_FONTSIZE)
    #ax.set_title("RQ3 summary: trend types by metric", fontsize=TITLE_FONTSIZE)
    ax.legend(fontsize=LEGEND_FONTSIZE)

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

    fig, ax = plt.subplots(figsize=(15, 6))

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
    ax.set_ylabel("number of systems", fontsize=AXIS_LABEL_FONTSIZE)
    #ax.set_title("RQ3: Number of systems per constraint level and setting", fontsize=TITLE_FONTSIZE)
    ax.legend(title="constraint level", fontsize=LEGEND_FONTSIZE,
    title_fontsize=LEGEND_TITLE_FONTSIZE,)

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

        fig, ax = plt.subplots(figsize=(11, 8))
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
        ax.set_ylabel("coverage value", fontsize=AXIS_LABEL_FONTSIZE)
        #ax.set_title(f"RQ3: Coverage distribution for {metric} by setting and constraint strength")

        if legend_handles:
            ax.legend(legend_handles, legend_labels, title="constraint strength", fontsize=LEGEND_FONTSIZE,
    title_fontsize=LEGEND_TITLE_FONTSIZE,)

        fig.tight_layout()
        fig.savefig(OUT_DIR / f"rq3_boxplots_{metric}.png", dpi=300, bbox_inches="tight")
        plt.close(fig)

def plot_rq3_boxplots_by_strength_and_source_t(rq3_raw: pd.DataFrame) -> None:
    strength_order = [
        Strength.WEAK.value,
        Strength.MODERATE.value,
        Strength.STRONG.value,
        Strength.VERYSTRONG.value,
    ]
    source_t_order = [1, 2, 3]
    coverage_t_order = [1, 2, 3]

    metric_order = sorted(rq3_raw["metric"].dropna().unique())

    metric_colors = {
        metric: color
        for metric, color in zip(
            metric_order,
            [
                "#332288",
                "#88CCEE",
                "#44AA99",
                "#117733",
                "#999933",
                "#DDCC77",
                "#CC6677",
                "#AA4499",
            ],
        )
    }

    for strength in strength_order:
        for source_t in source_t_order:
            subset = rq3_raw[
                (rq3_raw["constraint_level"] == strength) &
                (rq3_raw["sample_size_source_t"] == source_t)
            ].copy()

            if subset.empty:
                continue

            fig, ax = plt.subplots(figsize=(11, 8))

            base_positions = np.arange(len(coverage_t_order), dtype=float)

            n_metrics = len(metric_order)
            total_width = 0.8
            width = total_width / n_metrics
            offsets = np.linspace(
                -total_width / 2 + width / 2,
                total_width / 2 - width / 2,
                n_metrics,
            )

            legend_handles: list[Patch] = []

            for metric_idx, metric in enumerate(metric_order):
                data: list[list[float]] = []
                positions: list[float] = []

                for cov_idx, coverage_t in enumerate(coverage_t_order):
                    values = subset.loc[
                        (subset["metric"] == metric) &
                        (subset["coverage_t"] == coverage_t),
                        "coverage",
                    ].dropna().astype(float)

                    if values.empty:
                        continue

                    data.append(values.tolist())
                    positions.append(float(base_positions[cov_idx]) + float(offsets[metric_idx]))

                if not data:
                    continue

                box = ax.boxplot(
                    data,
                    positions=positions,
                    widths=width * 0.9,
                    patch_artist=True,
                    manage_ticks=False,
                )

                for patch in box["boxes"]:
                    patch.set_facecolor(metric_colors[metric])
                    patch.set_alpha(0.7)

                for median in box["medians"]:
                    median.set_color("black")

                legend_handles.append(
                    Patch(facecolor=metric_colors[metric], edgecolor="black", label=str(metric))
                )

            ax.set_xticks(base_positions)
            ax.set_xticklabels([str(t) for t in coverage_t_order])
            ax.set_xlabel("coverage $t_c$", fontsize=AXIS_LABEL_FONTSIZE)
            ax.set_ylabel("coverage value", fontsize=AXIS_LABEL_FONTSIZE)
            # ax.set_title(
            #     f"RQ3: Coverage distribution by metric "
            #     f"({strength}, sample size source t={source_t})", fontsize=TITLE_FONTSIZE
            # )

            if legend_handles:
                ax.legend(
                    handles=legend_handles,
                    title="metric",
                    bbox_to_anchor=(1.02, 1),
                    loc="upper left",fontsize=LEGEND_FONTSIZE,
    title_fontsize=LEGEND_TITLE_FONTSIZE,
                )

            fig.tight_layout()
            safe_strength = str(strength).replace(" ", "_").lower()
            fig.savefig(
                OUT_DIR / f"rq3_boxplots_{safe_strength}_{source_t}.png",
                dpi=300,
                bbox_inches="tight",
            )
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

    for raw in (rq2_raw, rq3_raw):
        if {"coverage_t", "sample_size_source_t"}.issubset(raw.columns):
            raw["setting"] = raw.apply(
                lambda row: add_setting_label(
                    int(row["coverage_t"]),
                    int(row["sample_size_source_t"]),
                ),
                axis=1,
            )

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

    export_all_legends(data["rq3_raw"])


    original_axes_legend = Axes.legend
    Axes.legend = lambda self, *args, **kwargs: None  

    try:
        plot_rq1_mean_rho_heatmap(data["rq1_summary"])
        plot_rq1_setting_heatmaps(data["rq1"])
        plot_rq1_mean_rho_bar(data["rq1_summary"])
        plot_rq1_setting_heatmaps_by_strategy(data["rq1"])
        plot_rq1_default_difference_boxplots(data["rq1_raw"])
        plot_rq1_setting_heatmaps_by_strategy_split(data["rq1"])
        plot_rq1_default_difference_boxplots_split(data["rq1_raw"])

        plot_rq2_best_strategy_heatmap(data["rq2"])
        plot_rq2_full_ordering_bars(data["rq2"])
        plot_rq2_median_bars(data["rq2"])
        plot_rq2_strategy_wins(data["rq2_summary"])
        plot_rq2_boxplots(data["rq2_raw"])
        plot_rq2_median_bars_split(data["rq2"])
        plot_rq2_boxplots_split(data["rq2_raw"])

        plot_rq3_trend_lines(data["rq3"])
        plot_rq3_weak_minus_verystrong_heatmap(data["rq3"])
        plot_rq3_trend_direction_summary(data["rq3_summary"])
        plot_rq3_boxplots(data["rq3_raw"])
        plot_rq3_system_counts_by_constraint_level(data["rq3_raw"])
        plot_rq3_boxplots_by_strength_and_source_t(data["rq3_raw"])
    finally:
        Axes.legend = original_axes_legend  

    print(f"Plots written to: {OUT_DIR.resolve()}")


if __name__ == "__main__":
    main()