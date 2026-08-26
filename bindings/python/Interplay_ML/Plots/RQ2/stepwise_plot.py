#

from pathlib import Path
from ...Sampling.Config import PyStrat, Techniques, Systems
from matplotlib.colors import LinearSegmentedColormap
import matplotlib.pyplot as plot 
import pandas as pd

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ2")

COLORMAP_GRID = LinearSegmentedColormap.from_list(
    "blue_red_div", ["#3A5A80", "#ffffff", "#B23A48"], N=256
)

STEPS = [
    ("0.01", "0.02"),
    ("0.02", "0.05"),
    ("0.05", "0.10"),
    ("0.10", "0.15"),
    ("0.15", "0.20")
]

STEP_LABELS = [f"{step1} -> {step2}" for step1, step2 in STEPS]

def getRQ2Res():
    df = pd.read_csv(INPUT_PATH / "rq2_summary_per_tech.csv")
    return df

def plot_steps(out):
    plot.rcParams.update({
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42,
    })

    df = getRQ2Res()
    mx = df.pivot(index="technique", columns="label", values="num_sig_bh").reindex(index=Techniques)

    fig, axes = plot.subplots(figsize=(10, 5))
    im = axes.imshow(mx.values, cmap=COLORMAP_GRID, vmin=0, vmax=10, aspect="auto")
    diff_mx = df.pivot(index="technique", columns="label", values="median_diff").reindex(index=Techniques)

    axes.set_xticks(range(len(STEP_LABELS)))
    axes.set_xticklabels(STEP_LABELS, rotation=45, ha="right")
    axes.set_yticks(range(len(Techniques)))
    axes.set_yticklabels(Techniques)
    axes.set_xlabel("Proportion transition")
    axes.set_ylabel("ML technique")
    axes.set_title("Number of systems with BH-significant MRE reduction per step", fontsize=10, weight="bold", pad=10)

    for i in range(len(Techniques)):
        for j in range(len(STEP_LABELS)):
            val = int(mx.values[i, j])
            diff = float(diff_mx.values[i, j])
            text_color = "white" if val > 5 else "black"
            axes.text(j, i, f"{val}\n Δ={diff:.3f}", ha="center", va="center", fontsize=10, color=text_color, weight="bold", linespacing=1)
    
    col_bar = fig.colorbar(im, ax=axes, shrink=1, pad=0.05)
    col_bar.set_label("Systems significantly improving", fontsize=10)
    col_bar.set_ticks([0, 2, 4, 6, 8, 10])

    fig.tight_layout()
    fig.savefig(out, format="pdf", bbox_inches="tight")
    plot.close(fig)


def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    plot_steps(PLOT_PATH / "step_mre.pdf")

if __name__ == "__main__":
    run()
