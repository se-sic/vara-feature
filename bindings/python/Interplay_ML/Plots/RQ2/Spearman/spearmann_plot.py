#

from pathlib import Path
from ....Sampling.Config import PyStrat, Techniques, Systems
from matplotlib.colors import LinearSegmentedColormap
from matplotlib.ticker import ScalarFormatter
from matplotlib.patches import Rectangle
import matplotlib.pyplot as plot 
import matplotlib.patches as patches
import pandas as pd
import numpy as np

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ2")

COLORMAP_GRID = LinearSegmentedColormap.from_list(
    "blue_red_div", ["#3A5A80", "#ffffff", "#B23A48"], N=256
)

def getRQ2Spearman(metric):
    df = pd.read_csv(INPUT_PATH / f"rq2_spearman_{metric}.csv")
    return df

def plot_spearman(metric, out):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    spaerman_df = getRQ2Spearman(metric)

    rho_mx = spaerman_df.pivot(index="system", columns="technique", values="rho").reindex(index=Systems, columns=Techniques)
    sig_mx = spaerman_df.pivot(index="system", columns="technique", values="sig").reindex(index=Systems, columns=Techniques)

    fig, axes = plot.subplots(figsize=(10, 8))

    vmin_data = rho_mx.values.min()
    vmax_data = rho_mx.values.max()
    im = axes.imshow(rho_mx.values, cmap=COLORMAP_GRID, vmin=vmin_data, vmax=vmax_data, aspect="auto") # type: ignore

    axes.set_xticks(range(len(Techniques)))
    axes.set_xticklabels(Techniques, rotation=45, ha="right")
    axes.set_yticks(range(len(Systems)))
    axes.set_yticklabels(Systems)
    axes.set_title(f"Spearman rho ({metric.upper()} vs. proportion)", fontsize=10, weight="bold", pad=8)

    for i, sys in enumerate(Systems):
        for j, tech in enumerate(Techniques):
            val = rho_mx.at[sys,tech]
            text_color = "black"
            axes.text(j, i, f"{val:.2f}", ha="center", va="center", fontsize=10, color=text_color)
            if not sig_mx.at[sys, tech]:
                axes.add_patch(Rectangle(
                    (j - 0.5, i -0.5), 1, 1, fill=False, edgecolor="black", linewidth=2, zorder=5
                ))

    col_bar = fig.colorbar(im, ax=axes, shrink=1, pad=0.05)
    col_bar.set_label(f"Spearman rho ({metric.upper()})", fontsize=10)

    fig.tight_layout()
    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    plot_spearman("mre", PLOT_PATH / "spearman_mre.pdf")
    plot_spearman("var", PLOT_PATH / "spearman_var.pdf")

if __name__ == "__main__":
    run()


