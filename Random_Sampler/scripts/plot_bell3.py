import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def plot_bell_numpy(csv_path):
    df = pd.read_csv(csv_path)
    data = df["ActiveFeatures"].values

    mu = np.mean(data)
    sigma = np.std(data)

    counts, bin_edges = np.histogram(data, bins=25, density=False)

    x = np.linspace(min(data), max(data), 200)
    gaussian = (1 / (sigma * np.sqrt(2 * np.pi))) * np.exp(-((x - mu)**2) / (2 * sigma**2))

    bin_width = bin_edges[1] - bin_edges[0]
    gaussian_scaled = gaussian * len(data) * bin_width

    plt.figure(figsize=(12, 10))

    plt.bar(
        bin_edges[:-1],
        counts,
        width=bin_width,
        align='edge',
        color="#ff69b4",
        edgecolor="black",
        alpha=0.7,
        label="Histogram (NumPy)"
    )

    plt.plot(
        x,
        gaussian_scaled,
        color="#aa00ff",
        linewidth=3,
        label=f"Gaussian Fit: μ={mu:.2f}, σ={sigma:.2f}"
    )

    plt.xlabel("Number of activated features", fontsize=14, fontweight="bold", color="#aa00ff")
    plt.ylabel("Frequency", fontsize=14, fontweight="bold", color="#aa00ff")
    plt.title("Bell Curve using NumPy + Matplotlib", fontsize=16, fontweight="bold", color="#ff69b4")

    plt.legend()
    plt.tight_layout()
    plt.show()
