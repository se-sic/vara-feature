import sys as sys
import pandas as pd 
import matplotlib.pyplot as plt

csv_path = sys.argv if len(sys.argv) > 1 else "output/frequency.csv"
out_path = sys.argv[2] if len(sys.argv) > 2 else None

df = pd.read_csv(csv_path)

plt.figure(figsize=(10,6))
plt.bar(df["label"], df["p_true"])
plt.xticks(rotation=90)
plt.ylabel("P(var) = true")
plt.title("Estimated Frequency of Variables")
plt.tight_layout()

if out_path:
    plt.savefig(out_path, dpi=200)
else:
    plt.show()