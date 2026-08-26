#

from pathlib import Path
from ....Sampling.Config import PyStrat, Techniques
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.pyplot as plot 
import pandas as pd
import numpy as np

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ1")

SAMPLE_SIZES = ["T1", "T2", "T3"]
COLORMAP_GRID = LinearSegmentedColormap.from_list(
    "blue_red_div", ["#3A5A80", "#ffffff", "#B23A48"], N=256
)

def getRQ1Wides(metric, t):
    df = pd.read_csv(INPUT_PATH / f"rq1_wide_{metric}_{t}.csv", index_col=0)
    return df

def getRQ1Posthoc(metric, t):
    path = INPUT_PATH / f"rq1_posthoc_{metric}_{t}.csv"
    if not path.exists():
        return {}
    df = pd.read_csv(path)
    out = {}
    for _, row in df.iterrows():
        comb1, comb2, sig_bh = row["comb1"], row["comb2"], bool(row["sig_bh"])
        out[(comb1, comb2)] = sig_bh
        out[(comb2, comb1)] = sig_bh 
    return out

def create_diagonals(wide_csv, tech, strat):
    cols = [f"{s}_{tech}" for s in strat]
    return wide_csv[cols].mean(axis=0).values.reshape(1, -1)

def create_offdiagonal(wide_csv, tech_row, tech_col, strat):
    cols_rows = [f"{s}_{tech_row}" for s in strat]
    cols_col = [f"{s}_{tech_col}" for s in strat]
    diff = wide_csv[cols_rows].values - wide_csv[cols_col].values
    return diff.mean(axis=0).reshape(1, -1)

def count_sig_bh(tech_row, tech_col, strat, posthoc):
    return sum(1 for s in strat if posthoc.get((f"{s}_{tech_row}", f"{s}_{tech_col}"), False))

def plot_kaltenecker(metric, t, strat, out):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    wide_df = getRQ1Wides(metric, t)
    posthoc_df = getRQ1Posthoc(metric, t)

    diag_df = np.concatenate([create_diagonals(wide_df, tech, strat).flatten() for tech in Techniques])
    diag_vmin, diag_vmax = float(diag_df.min()), float(diag_df.max())
    offdiag_df = np.concatenate([create_offdiagonal(wide_df, t_r, t_c, strat).flatten() for t_r in Techniques for t_c in Techniques if t_r != t_c])
    offdiag_max = float(np.quantile(np.abs(offdiag_df), 0.75))
    offdiag__vmin, offdiag_vmax = -offdiag_max, offdiag_max

    threshold = 4 if len(strat) == 6 else 3

    fig, axes = plot.subplots(len(Techniques), len(Techniques), figsize=(15,10))
    fig.subplots_adjust(wspace=0.1, hspace=0.1, left=0.10, right=0.85, top=0.85, bottom=0.15)
    fig.suptitle(f"{metric.upper()} - $t = {t[-1]}$", fontsize=10, weight="bold")

    diag_im = off_im = None

    for i, tech_row in enumerate(Techniques):
        for j, tech_col in enumerate(Techniques):
            ax = axes[i, j]

            if i==j:
                mx = create_diagonals(wide_df, tech_row, strat)
                diag_im = ax.imshow(mx, cmap="gray_r", vmin=diag_vmin, vmax=diag_vmax, aspect="auto")
            else:
                mx = create_offdiagonal(wide_df, tech_row, tech_col, strat)
                off_im = ax.imshow(mx, cmap=COLORMAP_GRID, vmin=offdiag__vmin, vmax=offdiag_vmax, aspect="auto")
                if count_sig_bh(tech_row, tech_col, strat, posthoc_df) >= threshold:
                    for spine in ax.spines.values():
                        spine.set_linewidth(4)
                        spine.set_edgecolor("black")

            if i==len(Techniques) - 1:
                ax.set_xticks(range(len(strat)))
                ax.set_xticklabels(strat,rotation=45, ha="right", fontsize=10)
                ax.xaxis.set_ticks_position("bottom")
                ax.xaxis.set_label_position("bottom")
            else:
                ax.set_xticks([])
            
            ax.set_yticks([])
            
    for i, tech in enumerate(Techniques):
        axes[i, 0].set_ylabel(tech, fontsize=11, rotation=0, ha="right", va="center", labelpad=15)
        axes[0, i].set_title(tech, fontsize=11, pad=8)

    diag_cax = fig.add_axes((0.9, 0.5, 0.015, 0.3))  
    diag_colbar = fig.colorbar(diag_im, cax=diag_cax) # type: ignore
    diag_colbar.set_label(f"{metric.upper()} (diagonal)", fontsize=9)

    offdiag_cax = fig.add_axes((0.9, 0.15, 0.015, 0.3))
    offdiag_colbar = fig.colorbar(off_im, cax=offdiag_cax) # type: ignore
    offdiag_colbar.set_label("row - column difference (off_diagonal)", fontsize=10)

    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    for t in SAMPLE_SIZES:
        plot_kaltenecker("mre", t, list(PyStrat.keys()), PLOT_PATH / f"kaltenecker_mre_{t}.pdf")
        plot_kaltenecker("var", t, [s for s in PyStrat if s not in {"twise", "sbs"}], PLOT_PATH / f"kaltenecker_var_{t}.pdf")

if __name__ == "__main__":
    run()
