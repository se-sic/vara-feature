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

SAMPLE_SIZES = ["T1", "T2", "T3"]
TECHNIQUE_COLORS = {
    "CART": "#4C72B0",
    "kNN":  "#DD8452",
    "KRR":  "#55A467",
    "MR":   "#C44E52",
    "RF":   "#8172B2",
    "SVR":  "#937860",
}

TECHNIQUE_SIGNS = {
    "CART": "o",
    "kNN":  "s",
    "KRR":  "D",
    "MR":   "^",
    "RF":   "v",
    "SVR":  "P",
}

def getRQ1Wides(metric, t):
    df = pd.read_csv(INPUT_PATH / f"rq1_wide_{metric}_{t}.csv", index_col=0)
    return df

def getComboData(metric, strats):
    data = {tech: {} for tech in Techniques}
    for size in SAMPLE_SIZES:
        df_wide = getRQ1Wides(metric, size)
        for tech in Techniques:
            cols = [f"{s}_{tech}" for s in strats if f"{s}_{tech}" in df_wide.columns]
            values = df_wide[cols].values.flatten()
            data[tech][size] = values
    return data

def plot_line_evolution(metric, strats, out, use_log):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    data = getComboData(metric, strats)

    fig, axes = plot.subplots(figsize=(8, 3))
    x = list(range(len(SAMPLE_SIZES)))

    for tech in Techniques:
        medians = [np.median(data[tech][t]) for t in SAMPLE_SIZES]
        q25 = [np.quantile(data[tech][t], 0.25) for t in SAMPLE_SIZES]
        q75 = [np.quantile(data[tech][t], 0.75) for t in SAMPLE_SIZES]
        color = TECHNIQUE_COLORS[tech]
        sign = TECHNIQUE_SIGNS[tech]

        #axes.fill_between(x, q25, q75, color=color, alpha=0.15, zorder=1)
        axes.plot(x, medians, marker=sign, color=color, label=tech, linewidth=2, markersize=5, zorder=3)

    if use_log:
        axes.set_yscale("log")
        axes.yaxis.set_major_formatter(ScalarFormatter())
    
    axes.set_xticks(x)
    axes.set_xticklabels([f"$t= {i+1}$" for i in x])
    axes.set_xlabel("Reference sample size")
    axes.set_ylabel(f"{metric.upper()} (median across systems and strategies)")
    axes.set_xlim(-0.2, len(SAMPLE_SIZES) - 1)
    axes.grid(axis="y", linestyle=":", linewidth=0.5, alpha=0.5)
    axes.spines["top"].set_visible(False)
    axes.spines["right"].set_visible(False)
    axes.legend(loc="center left", bbox_to_anchor=(1.02, 0.5), frameon=False, ncol=2, fontsize=10)

    fig.tight_layout()
    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    plot_line_evolution("mre", PyStrat.keys(), PLOT_PATH / "evolution_mre.pdf", use_log=False)
    plot_line_evolution("var", strats=[strat for strat in PyStrat if strat not in {"twise", "sbs"}], out=PLOT_PATH / "evolution_var.pdf", use_log=True)

if __name__ == "__main__":
    run()
