import pandas as pd
import numpy as np
from scipy import stats
from pathlib import Path 
import scikit_posthocs as sp
from statsmodels.stats.multitest import multipletests

from ..Sampling.Config import TSize

OUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")

def friedman(res_csv):
    #determines whether at least two combinations differ from one another - but does not tell which ones
    num_rows, num_col = res_csv.shape
    stat_chi2, p = stats.friedmanchisquare(*[res_csv[col].to_numpy() for col in res_csv.columns])
    w_kendall = stat_chi2/(num_rows * (num_col-1))

    f_stats = (stat_chi2 * (num_rows-1)) / (num_rows * (num_col-1) - stat_chi2)
    df1 = num_col - 1 
    df2 = (num_col - 1) * (num_rows - 1)
    new_p_f = stats.f.sf(f_stats, df1, df2)
    
    return {"chi2": stat_chi2, "chi2_p": p, "un_rows": num_rows, "un_cols": num_col, "w_kendall": w_kendall, 
            "f_imandav": f_stats, "p_imandav": new_p_f, "df1": df1, "df2": df2}

def wilcoxon_holm_bh(df):
    #determines which pairs are significantly different - but doe snot tell you which one is better
    groups = [df[col].to_numpy() for col in df.columns]
    wilcoxon_matrix = sp.posthoc_wilcoxon(groups, p_adjust=None)
    wilcoxon_matrix.index = df.columns
    wilcoxon_matrix.columns = df.columns

    rows = []
    columns = list(df.columns)
    for i, comb1 in enumerate(columns):
        for j in range(i+1, len(columns)):
            comb2 = columns[j]
            rows.append({
                "comb1": comb1, 
                "comb2": comb2,
                "p": float(wilcoxon_matrix.iloc[i, j]), # type: ignore
                "median_diff_combs": float(np.median(df[comb1]-df[comb2])),
            })
    new_df = pd.DataFrame(rows)

    p_df = new_df["p"].to_numpy()
    _, p_holm, _, _ = multipletests(p_df, alpha=0.05, method="holm") # type: ignore
    _, p_bh, _, _ = multipletests(p_df, alpha=0.05, method="fdr_bh") # type: ignore
    new_df["p_holm"] = p_holm
    new_df["p_bh"] = p_bh
    new_df["sig_holm"] = new_df["p_holm"] < 0.05
    new_df["sig_bh"] = new_df["p_bh"] < 0.05
    
    return new_df.sort_values("p_bh").reset_index(drop=True)

def mean_ranking(df):
    ranks = df.rank(axis=1, method="average", ascending=True)
    new_df = pd.DataFrame({
        "mean_ranks": ranks.mean(axis=0),
        "std_rank": ranks.std(axis=0, ddof=1),
    })
    return new_df.sort_values("mean_ranks")

def run_metric(metric, out):
    rows = []
    for s_type in TSize:
        size_key = f"T{s_type}"
        out_path = OUT_PATH / out.format(size=size_key)
        df = pd.read_csv(out_path, index_col=0)
        if df.isna().any().any():
            raise Exception(F"NaNs in {out_path}")
        
        fried = friedman(df)

        mean_ranking(df).to_csv(OUT_PATH / f"rq1_ranks_{metric}_{size_key}.csv")

        if fried["p_imandav"] < 0.05:
            wilcoxon_holm_bh(df).to_csv(OUT_PATH / f"rq1_posthoc_{metric}_{size_key}.csv", index=False)

        rows.append({
            "metric": metric,
            "size": size_key,
            **fried,
            "posthoc_written": fried["p_imandav"] < 0.05
        })

    return pd.DataFrame(rows)
    
def run():
    OUT_PATH.mkdir(parents=True, exist_ok=True)
    mre_res = run_metric("mre", "rq1_wide_mre_{size}.csv")
    var_res = run_metric("var", "rq1_wide_var_{size}.csv")
    comb_summary = pd.concat([mre_res, var_res], ignore_index=True)
    comb_summary.to_csv(OUT_PATH / "rq1_stats_res.csv", index=False)
    with pd.option_context("display.width", 180, "display.max_columns", 20):
        print(comb_summary.to_string(index=False))

if __name__ == "__main__":
    run()

