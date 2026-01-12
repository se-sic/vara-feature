import pandas as pd 
import matplotlib.pyplot as plt

def plot_deviation(csv_path,):
    df = pd.read_csv(csv_path)

    df.columns = df.columns.str.strip()

    df["Prob"] = df["Prob"].astype(float)

    df["SampledPercent"] = df["Prob"] * 100.0

    df = df.sort_values("Label")

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

    mean_val = df["SampledPercent"].mean()
    df["Deviation"] = df["SampledPercent"] - mean_val

    bars = plt.barh(
        df["Label"], 
        df["Deviation"],
        color = colors[3],
        edgecolor = 'black',
        alpha = 0.9,
        label = 'Deviation around mean'
    )

    for bar, value in zip(bars, df["Deviation"]):
        width = bar.get_width()
        y_pos = bar.get_y() + bar.get_height() / 2
        plt.text(
            width + 1,
            y_pos,
            "",
            va="center",
            ha="left",
            fontsize=10, 
            color=colors[8], 
            fontweight="bold"
        )

    plt.axvline(
        0,
        color = colors[8],
        linestyle='--',
        linewidth=2,
        label=f"Mean = {mean_val:.2f}%"
    )

    plt.xlabel("Deviation around mean", fontsize=14,
               fontweight='bold', color=colors[8])
    plt.ylabel("Feature", fontsize=14, fontweight='bold', color=colors[8])
    plt.title("Uniformity of Feature Activation Frequencies",
              fontsize=16, fontweight='bold', color='#ff1493')

    plt.grid(axis='x', linestyle='--', alpha=0.7, color=colors[4])

    plt.legend(loc="upper right")

    plt.tight_layout()

    plt.show()
