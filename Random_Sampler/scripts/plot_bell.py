import sys 
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

def plot_bell(csv_path):
    df = pd.read_csv(csv_path)

    data = df["ActiveFeatures"]

    plt.figure(figsize=(12, 10))

    pink = "#ff69b4"
    purple = "#aa00ff"

    plt.hist(
        data,
        bins=25,
        edgecolor='black',
        color=pink,
        alpha=0.7,
    )

    mu = np.mean(data)
    sigma = np.std(data)

    xmin, xmax = min(data), max(data)
    x = np.linspace(xmin, xmax, 100)
    y = (1 / (sigma * np.sqrt(2 * np.pi))) * np.exp(-((x - mu)**2 / (2 * sigma ** 2)))
    y = y * len(data) * (xmax - xmin) / 25

    plt.plot(x, y, color='purple', lw=3, label=f"Normal approx μ={mu:.2f}, σ={sigma:.2f}")
    plt.xlabel("Number of activated features", fontsize=14, fontweight="bold", color=purple)
    plt.ylabel("Frequency", fontsize=14, fontweight="bold", color=purple)
    plt.legend()
    plt.tight_layout()

    plt.show()