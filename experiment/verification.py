import pandas as pd

df = pd.read_csv("results/rq3_spearman_results_new.csv")

spread = (df.groupby(["coverage_t", "sample_size_source_t"])["spearman_rho"]
            .agg(["min", "max", "mean"]))
spread["spread"] = spread["max"] - spread["min"]
spread["mean_abs"] = spread["mean"].abs()  

print(spread.sort_values("spread").round(3))