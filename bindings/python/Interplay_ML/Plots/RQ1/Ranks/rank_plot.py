# Whilst Kendall's W communicates a consesus on a specific combination of sampling strategy and machien learning technique, teh mean rank
# displays which combinations are considered in that considered ranking

from pathlib import Path
from ....Sampling.Config import PyStrat, Techniques
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.pyplot as plot 
import pandas as pd

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ1")

SAMPLE_SIZES = ["T1", "T2", "T3"]
COLORMAP_GRID = LinearSegmentedColormap.from_list(
    "blue_red_div", ["#3A5A80", "#ffffff", "#B23A48"], N=256
)
def getRQ1Ranks(metric, t):
    input = INPUT_PATH / f"rq1_ranks_{metric}_{t}.csv"
    df = pd.read_csv(input, index_col=0)
    df.index.name = "comb"
    df = df.reset_index()
    df[["strategy", "technique"]] = df["comb"].str.split("_", n=1, expand=True)
    return df.pivot(index="strategy", columns="technique", values="mean_ranks")

def plot_ranks(metric, strats, out):
    plot.rcParams.update({
        "font.family": "serif", # type: ignore
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    num_cells = len(strats) * len(Techniques)
    fig_height = 4 if len(strats) == 6 else 3

    fig, axes = plot.subplots(1, 3, figsize=(10, fig_height), constrained_layout=True)
    matrices = [getRQ1Ranks(metric, t).reindex(index=strats, columns=Techniques) for t in SAMPLE_SIZES] # type: ignore
    vmin = 1
    vmax = num_cells
    im = None

    for ax, t, mx in zip(axes, SAMPLE_SIZES, matrices):
        im = ax.imshow(mx.values, cmap=COLORMAP_GRID, vmin=vmin, vmax=vmax, aspect="auto")
        ax.set_xticks(range(len(Techniques)))
        ax.set_xticklabels(Techniques, rotation=45, ha="right")
        ax.set_yticks(range(len(strats)))
        ax.set_yticklabels(strats)
        ax.set_title(f"$t = {t[-1]}$", fontsize=10)

        for strat in range(len(strats)):
            for tech in range(len(Techniques)):
                val = mx.values[strat, tech]
                text_color = "white" if val > (vmin + vmax) / 2 else "black"
                ax.text(tech, strat, f"{val:.1f}", 
                          ha="center",
                          va="center",
                          fontsize=10,
                          color=text_color)
                
    axes[0].set_ylabel("Sampling Strategy")
    for a in axes:
        a.set_xlabel("ML Technique")
        
    colbar = fig.colorbar(im, ax=axes, shrink=0.5, pad=0.02) # type: ignore
    metric_label = r"$\overline{MRE}$" if metric == "mre" else r"$Var$"
    colbar.set_label(f"Mean rank ({metric_label}, 1= best, 36=worst)")

    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    plot_ranks("mre", PyStrat.keys(), PLOT_PATH / "rank_mre.pdf")
    plot_ranks("var", strats=[strat for strat in PyStrat if strat not in {"twise", "sbs"}], out=PLOT_PATH / "rank_var.pdf")
    
if __name__ == "__main__":
    run()

