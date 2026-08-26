# Kendall's W determines the effect size of how much teh subjects systems agree on one combination between sampling strategy and machine learning 
# technique w.r.t. Accuaracy (MRE) and Stability (VAR). Cohen's Thresholds are used to classify the results of the statitsical value.
# There are three classes in total, each indicating teh gravity of the effect, i.e. degree of how much all subject systems agree on one good/bad combination.
# The result show that a combination effect is undeniable and at that, that there is a conses on one bad and good combinations. This consens is
# strengthened with rising sample size.
# This way W tells us if we can be confident in that ranking

from pathlib import Path 
import  matplotlib.pyplot as plot 
import pandas as pd

INPUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out/rq1_stats_res.csv")
PLOT_PATH = Path("bindings/python/Interplay_ML/Plots/RQ1")

SAMPLE_SIZES = ["T1", "T2", "T3"]
COHEN_BENCHMARK = [(0.10, "small"), (0.30, "moderate"), (0.5, "strong")]

def getRQ1Res():
    df = pd.read_csv(INPUT_PATH)
    if "w_kendall" not in df.columns: 
        raise Exception(f"w_kendall not in {INPUT_PATH}")
    return df[["metric", "size", "w_kendall"]].copy()

def plot_kendall(df, Out):
    plot.rcParams.update({
        "font.family": "serif", # type: ignore
        "font.size": 10,
        "axes.labelsize": 10,
        "legend.fontsize": 10,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "pdf.fonttype": 42.
    })
    fig, axes = plot.subplots()

    x = list(range(len(SAMPLE_SIZES)))

    for metric, marker, color, label, y_off in [("mre", "o", "#B23A48", r"$\overline{MRE}$", 18), ("var", "s", "#3A5A80", r"$Var$", 10)]:
        y = (df[df["metric"] == metric].set_index("size").reindex(SAMPLE_SIZES)["w_kendall"].to_numpy())
        axes.plot(x, y, marker=marker, color=color, label=label, linewidth=2, markersize=6, zorder=3)
        for x_i, y_i in zip(x, y):
            axes.annotate(f"{y_i:.2f}", (x_i, y_i), textcoords="offset points", xytext=(0,y_off), fontsize=10, color=color, zorder=4)
    
    y_axis = axes.get_yaxis_transform()
    for bench, name in COHEN_BENCHMARK:
        axes.axhline(bench, color="gray", linewidth=0.5, alpha=0.6, zorder=1, linestyle="--")
        axes.text(0.01, bench - 0.02, name, fontsize=10, color="gray", ha="left", alpha=0.9, transform=y_axis)

    axes.set_xticks(x)
    axes.set_xticklabels([f"$t= {i+1}$" for i in x])
    axes.set_xlabel("Reference sample size")
    axes.set_ylabel("Kendall's $W$")
    axes.set_ylim(0, 1)
    axes.set_xlim(-0.3, len(SAMPLE_SIZES)-0.7)
    axes.legend(loc="center right", frameon=False)
    axes.grid(axis="y", linestyle=":", linewidth=0.4)
    axes.spines["top"].set_visible(False)
    axes.spines["right"].set_visible(False)

    fig.tight_layout()
    fig.savefig(Out, format="pdf", bbox_inches="tight")
    plot.close(fig)

def run():
    PLOT_PATH.mkdir(parents=True, exist_ok=True)
    df = getRQ1Res()
    out = PLOT_PATH / "kendall_w.pdf"
    plot_kendall(df, out)
    
if __name__ == "__main__":
    run()


