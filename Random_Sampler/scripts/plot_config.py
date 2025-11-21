import pandas as pd
import matplotlib.pyplot as plt

def plot_config_frequency(csv_path):
    df = pd.read_csv(csv_path)
    plt.figure(figsize=(16, 7))

    plt.bar(
        df["ConfigID"],
        df["Count"],
        color="#ff69b4",
        edgecolor="black",
        alpha=0.8
    )

    plt.xlabel("Configuration ID", fontsize=14, fontweight="bold", color="#aa00ff")
    plt.ylabel("Frequency", fontsize=14, fontweight="bold", color="#aa00ff")
    plt.title("Configuration Frequency Distribution", fontsize=16, fontweight="bold", color="#ff69b4")

    plt.tight_layout()
    plt.show()