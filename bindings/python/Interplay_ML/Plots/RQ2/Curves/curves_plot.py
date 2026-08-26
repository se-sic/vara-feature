#

from pathlib import Path
from ....Sampling.Config import PyStrat, Techniques, Systems
from matplotlib.ticker import ScalarFormatter
import matplotlib.pyplot as plot 
import matplotlib.patches as patches
import pandas as pd
import numpy as np

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ2")

SKIPPED_SYSTEM = {"VP9", "JavaGC"}
SKIPPED_TECHS = {"kNN", "KRR", "SVR"}
SKIPPED_PROPS = {0.15, 0.20}

SYSTEM_COLORS = {
    "7z":           "#4C72B0",
    "BerkeleyDBC":  "#DD8452",
    "Dune":         "#55A467",
    "Hippacc":      "#C44E52",
    "Irzip":        "#8172B2",
    "JavaGC":       "#937860",
    "LLVM":         "#DA8BC3",
    "Polly":        "#8C8C8C",
    "VP9":          "#CCB974",
    "x264":         "#64B5CD",
}

def getRQ2Res(metric):
    df = pd.read_csv(INPUT_PATH / f"rq2_{metric}.csv")
    return df

def check_skipped(system, tech, prop):
    return (system in SKIPPED_SYSTEM
            and tech in SKIPPED_TECHS
            and prop in SKIPPED_PROPS
            )

def plot_curves(metric, out, use_log):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    df = getRQ2Res(metric)

    fig, axes = plot.subplots(2, 3, figsize=(10, 8), sharex=True)
    axes = axes.flatten()

    for ax, tech in zip(axes, Techniques):
        df_tech = df[df["technique"] == tech]

        for sys in Systems:
            df_sys = df_tech[df_tech["system"] == sys].copy()
            df_sys = df_sys[~df_sys.apply(lambda r: check_skipped(r["system"], r["technique"], r["proportion"]), axis=1)]
            df_sys = df_sys.sort_values("proportion")

            if len(df_sys) == 0:
                continue

            ax.plot(df_sys["proportion"], df_sys[metric], marker="o", markersize=3, linewidth=1, color=SYSTEM_COLORS[sys], label=sys, alpha=0.5)
            ax.set_title(tech, fontsize=10, weight="bold")
            ax.grid(axis="y", linestyle=":", linewidth=0.5, alpha=0.5)
            ax.spines["top"].set_visible(True)
            ax.spines["right"].set_visible(True)

            if use_log:
                ax.set_yscale("log")
                ax.yaxis.set_major_formatter(ScalarFormatter())
    
    for ax in axes[3:]:
        ax.set_xlabel("Proportion of valid conifguration space")
    for i in (0, 3):
        axes[i].set_ylabel(metric.upper())
    
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc="center left", bbox_to_anchor=(1, 0.5), frameon=False, fontsize=10, title="System")
    fig.tight_layout(rect=(0, 0, 1, 1))
    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)


def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    plot_curves("mre", PLOT_PATH / "curves_mre.pdf", use_log=False)
    plot_curves("var", PLOT_PATH / "curves_var.pdf", use_log=True)

if __name__ == "__main__":
    run()