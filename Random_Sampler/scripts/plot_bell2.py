import sys 
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

def plot_bell_saeborn(csv_path):
    df = pd.read_csv(csv_path)
    data = df["ActiveFeatures"]

    sns.set_theme(style="whitegrid")

    plt.figure(figsize=(12, 10))

    pink = "#ff69b4"
    purple = "#aa00ff"

    sns.histplot(
        df["ActiveFeatures"],
        bins=25,
        kde=True,
        color=pink,
        edgecolor='black',
        alpha=0.7,
    )

    # mu = np.mean(data)
    # sigma = np.std(data)
    # xmin, xmax = min(data), max(data)
    # x = np.linspace(xmin, xmax, 200)
    # y = (1 / (sigma * np.sqrt(2 * np.pi))) * np.exp(-((x - mu)**2 / (2 * sigma ** 2)))
    # y = y * len(data) * (xmax - xmin) / 25

    # plt.plot(
    #     x, y, 
    #     color=purple, 
    #     lw=3, 
    #     label=f"Normal approx μ={mu:.2f}, σ={sigma:.2f}"
    # )

    plt.xlabel("Number of activated features", fontsize=14, fontweight="bold", color=purple)
    plt.ylabel("Frequency", fontsize=14, fontweight="bold", color=purple)
    plt.title("Normal Distribution Approximation with Seaborn", fontsize=16, fontweight="bold", color=purple)
    plt.legend()
    plt.tight_layout()
    plt.show()