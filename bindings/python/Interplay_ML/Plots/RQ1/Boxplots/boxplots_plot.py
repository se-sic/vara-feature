#

from pathlib import Path
from ....Sampling.Config import PyStrat, Techniques
from matplotlib.ticker import ScalarFormatter
import matplotlib.pyplot as plot 
import matplotlib.patches as patches
import pandas as pd
import numpy as np

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ1")

SAMPLE_SIZES: list[str] = ["T1", "T2", "T3"]
TECHNIQUE_COLORS = {
    "CART": "#4C72B0",
    "kNN":  "#DD8452",
    "KRR":  "#55A467",
    "MR":   "#C44E52",
    "RF":   "#8172B2",
    "SVR":  "#937860",
}

def getRQ1Wides(metric, t):
    df = pd.read_csv(INPUT_PATH / f"rq1_wide_{metric}_{t}.csv", index_col=0)
    return df

def getComboData(wide_df, strats):
    combos = {}
    for strat in strats:
        for tech in Techniques:
            col = f"{strat}_{tech}"
            if col in wide_df.columns:
                combos[f"{strat}_{tech}"] = wide_df[col].values
    return combos

def plot_boxes(metric, t, strats, out):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    df_wide = getRQ1Wides(metric, t)
    combos = getComboData(df_wide, strats)

    sorted_names = sorted(combos.keys(), key=lambda c: np.mean(combos[c]))
    sorted_values = [combos[name] for name in sorted_names]
    sorted_colors = [TECHNIQUE_COLORS[name.split("_", 1)[1]] for name in sorted_names]

    num_combos = len(sorted_names)
    fig, axes = plot.subplots(figsize=(max(10, num_combos * 0.35), 5))

    box_plot = axes.boxplot(
        sorted_values,
        positions=range(num_combos),
        widths=0.7,
        patch_artist=True,
        showfliers=True,
        flierprops=dict(marker="o", markersize=4, markerfacecolor="black", markeredgecolor="none"),
        medianprops=dict(color="black", linewidth=1),
        boxprops=dict(linewidth=0.5),
        whiskerprops=dict(linewidth=0.5),
        capprops=dict(linewidth=0.5),
    )

    for patch, color in zip(box_plot["boxes"], sorted_colors):
        patch.set_facecolor(color)
        patch.set_alpha(0.75)

    axes.set_xticks(range(num_combos))
    axes.set_xticklabels(sorted_names, rotation=45, fontsize=8)
    axes.set_yscale("log")
    axes.yaxis.set_major_formatter(ScalarFormatter())
    axes.set_ylabel(f"{metric.upper()} (log scale)")
    axes.set_title(f"{metric.upper()} - $t = {t[-1]}$", fontsize=10, weight="bold")
    axes.grid(axis="y", linestyle=":", linewidth=0.5, alpha=0.5)
    axes.spines["top"].set_visible(False)
    axes.spines["right"].set_visible(False)

    handles = [patches.Patch(color=TECHNIQUE_COLORS[tech], alpha=0.75, label=tech) for tech in Techniques]
    axes.legend(handles=handles, title="Technique", loc="center left", fontsize=10, title_fontsize=10, frameon=False, bbox_to_anchor=(1.02, 0.5))

    fig.tight_layout()
    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    for t in SAMPLE_SIZES:
        plot_boxes("mre", t, PyStrat.keys(), PLOT_PATH / f"boxplots_mre_{t}.pdf")
        plot_boxes("var", t, strats=[strat for strat in PyStrat if strat not in {"twise", "sbs"}], out=PLOT_PATH / f"boxplots_var_{t}.pdf")

if __name__ == "__main__":
    run()
