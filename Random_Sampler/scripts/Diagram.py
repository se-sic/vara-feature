import pandas as pd 
import matplotlib.pyplot as plt

def plot_frequency(csv_path, out_path=None):
    df = pd.read_csv(csv_path)

    df.columns = df.columns.str.strip()

    df["Prob"] = df["Prob"].astype(float)
    df["TheoProb"] = df["TheoProb"].astype(float)

    df["SampledPercent"] = df["Prob"] * 100.0
    df["TheoPercent"] = df["TheoProb"] * 100.0

    df = df.sort_values("SampledPercent", ascending=True)

    colors = [
        "#ff69b4",  # slut pop pink
        "#ff1493",  # deep pink
        "#c71585",  # medium violet-pink
        "#ff6fff",  # bubblegum
        "#ff85a1",  # soft rose
        "#ff4fc4",  # candy magenta
        "#ff3ea5",  # neon berry
        "#d100a6",  # diva plum
        "#aa00ff"   # slut goth purple
    ]

    plt.figure(figsize=(14, 12))

    n = len(df)
    base_y = list(range(n))

    offset = 0.25
    y_sampled = [y - offset for y in base_y]
    y_theo = [y + offset for y in base_y]


    bars_sampled = plt.barh(
        y_sampled, 
        df["SampledPercent"],
        color = colors[3],
        edgecolor = 'black',
        alpha = 0.9,
        height= 0.4,
        label = 'Sampled'
    )

    bars_theo = plt.barh(
        y_theo, 
        df["TheoPercent"],
        color = colors[8],
        edgecolor = 'black',
        alpha = 0.6,
        height=0.4,
        label= "Theoretical"
    )

    for y, bar, value in zip(y_sampled, bars_sampled, df["SampledPercent"]):
        width = bar.get_width()
        plt.text(
            width + 1,
            y,
            f"{value:.1f}%",
            va="center",
            ha="left",
            fontsize=10, 
            color="#d100a6", 
            fontweight="bold"
        )

    y_ticks = base_y
    plt.yticks(
        y_ticks,
        df["Label"],
        fontsize=12,
        fontweight="bold",
        color=colors[8]
    )

    plt.xlabel("Selection frequency [%]", fontsize=14,
               fontweight='bold', color=colors[8])
    plt.ylabel("Feature", fontsize=14, fontweight='bold', color=colors[8])
    plt.title("Theoretical vs Sampled Feat Probs",
              fontsize=16, fontweight='bold', color='#ff1493')

    plt.grid(axis='x', linestyle='--', alpha=0.7, color=colors[4])

    plt.legend(loc="lower right")

    plt.tight_layout()

    if out_path:
        plt.savefig(out_path, dpi=300)

    plt.show()
