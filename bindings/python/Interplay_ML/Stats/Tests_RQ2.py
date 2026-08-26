import pandas as pd
import numpy as np
from scipy import stats
from pathlib import Path
from ..Sampling.Config import Proportions, Systems, Results
from .Aggregate_RQ2 import prop_to_str
from statsmodels.stats.multitest import multipletests


OUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
SORT_PROPS = sorted(Proportions)
PROP_PAIRS = [(SORT_PROPS[i], SORT_PROPS[i+1]) for i in range(len(SORT_PROPS)-1)]
PROP_LABELLED_PAIRS = [f"{prop_to_str(p1)}->{prop_to_str(p2)}" for p1, p2 in PROP_PAIRS]

def spearman(df, value_cols):
    rows = []
    for (sys, tech), g in df.groupby(["system", "technique"]):
        g = g.sort_values("proportion")
        num_rows = len(g)
        if num_rows < 3:
            raise Exception(f"{sys}/{tech}: only {num_rows} many proportions, Spearman cant be computed")
        res = stats.spearmanr(g["proportion"].to_numpy(), g[value_cols].to_numpy())
        rho, p = float(res.statistic), float(res.pvalue) # type:ignore
        rows.append({
            "system": sys,
            "technique": tech,
            "num_rows": num_rows,
            "rho": rho,
            "p": p,
            "sig": p < 0.05,
        })
    
    return pd.DataFrame(rows).sort_values(["technique", "system"]).reset_index(drop=True)

def summarize_tech(df):
    rows = []
    for tech, g in df.groupby("technique"):
        rhos = g["rho"].to_numpy()
        num_sys = len(rhos)

        res = stats.wilcoxon(rhos, alternative="less")
        statistic, p = float(res.statistic), float(res.pvalue) # type:ignore

        rows.append({
            "technique": tech,
            "num_sys": num_sys,
            "median_rho": float(np.median(rhos)),
            "mean_rho": float(np.mean(rhos)),
            "min_rho": float(np.min(rhos)),
            "max_rho": float(np.max(rhos)),
            "num_neg": int((rhos < 0).sum()),
            "num_sig_neg": int(((rhos < 0) & (g["p"].to_numpy() < 0.05)).sum()),
            "wilcoxon_statistic": statistic,
            "wilcoxon_p": p,
            "sig_decrease": p < 0.05,
        })

    return pd.DataFrame(rows).sort_values("median_rho").reset_index(drop=True)
    
def run_spearman():
    for metric, file in [("mre", "rq2_mre.csv"), ("var", "rq2_var.csv")]:
        metric_res = pd.read_csv(OUT_PATH / file)
        if metric_res.isna().any().any():
            raise Exception(f"NaNs in rq2_{metric}.csv")
        spearman_res = spearman(metric_res, metric)
        spearman_res.to_csv(OUT_PATH / f"rq2_spearman_{metric}.csv", index=False)

        summary_per_tech = summarize_tech(spearman_res)
        summary_per_tech.to_csv(OUT_PATH / f"rq2_spearman_per_technique_{metric}.csv", index=False)

        with pd.option_context("display.width", 200, "display.max_columns", 20, "display.float_format", "{:.4f}".format):
            print(summary_per_tech.to_string(index=False))
    
        print(f"Cells with rho < 0: {(spearman_res['rho'] < 0).sum()}/{len(spearman_res)}")
        print(f"Cells with rho < 0 and p < 0.05: {((spearman_res['rho'] < 0) & spearman_res['sig']).sum()}/{len(spearman_res)}")
        print(f"Cells with > 0: {(spearman_res['rho'] > 0).sum()}/{len(spearman_res)}")
        print(f"Cells with num_rows = 4: {(spearman_res['num_rows'] == 4).sum()}/{len(spearman_res)}")


def is_capped(sys, tech, prop1, prop2):
    if sys not in {"VP9", "JavaGC"} or tech not in {"kNN", "KRR", "SVR"}:
        return False
    return prop1 in {0.15, 0.20} or prop2 in {0.15, 0.20}

def getRQ2Files():
    frames = []
    for sys in Systems:
        file = Results / f"rq_2_{sys}_random.csv"
        if not file.exists():
            raise FileNotFoundError(file)
        frames.append(pd.read_csv(file))
    return pd.concat(frames, ignore_index=True)

def pairwise_wilcoxon_test(df):
    wide_rq2 = (df.pivot_table(index=["system", "technique", "seed"],
                               columns="size_type", values="mre", aggfunc="first").reset_index())
    rows = []
    for (sys, tech), g in wide_rq2.groupby(["system", "technique"]):
        g = g.sort_values("seed")
        for (p1, p2), label in zip(PROP_PAIRS, PROP_LABELLED_PAIRS):
            if is_capped(sys, tech, p1, p2):
                rows.append({"system": sys, "technique": tech, "label": label,
                             "num_pairs": 0, "median_diff_pairs": np.nan,
                             "stats": np.nan, "p": np.nan, "capped": True})
                continue
            df_prop1 = g[prop_to_str(p1)].to_numpy()
            df_prop2 = g[prop_to_str(p2)].to_numpy()
            if np.isnan(df_prop1).any() or np.isnan(df_prop2).any():
                raise Exception(f"NaN in {sys}/{tech} at {label}")
            diff = df_prop2 - df_prop1
            if np.all(diff == 0):
                stat_val, p = np.nan, 1.0
            else:
                res = stats.wilcoxon(df_prop2, df_prop1, alternative="less", zero_method="wilcox") # type: ignore
                stat_val, p = float(res.statistic), float(res.pvalue) # type: ignore
            
            rows.append({"system": sys, "technique": tech, "label": label, "num_pairs": len(diff), 
                         "median_diff_pairs": float(np.median(diff)), "stats": stat_val, "p": p, "capped": False})
        
    new_df = pd.DataFrame(rows)
    df_non_capped = new_df[~new_df["capped"]].copy()
    wilcoxon_p = df_non_capped["p"].to_numpy()
    _, p_holm, _, _ = multipletests(wilcoxon_p, alpha=0.05, method="holm")
    _, p_bh, _, _ = multipletests(wilcoxon_p, alpha=0.05, method="fdr_bh")
    df_non_capped["p_holm"] = p_holm
    df_non_capped["p_bh"] = p_bh
    df_non_capped["sig_holm"] = df_non_capped["p_holm"] < 0.05
    df_non_capped["sig_bh"] = df_non_capped["p_bh"] < 0.05
    new_df = new_df.merge(df_non_capped[["system", "technique", "label", "p_holm", "p_bh", "sig_holm", "sig_bh"]], 
                            on=["system", "technique", "label"], how="left")
        
    return new_df.sort_values(["technique", "label", "system"]).reset_index(drop=True)
    
def summarize_pair_tech(df):
    label_order = {label: idx for idx, label in enumerate(PROP_LABELLED_PAIRS)}
    rows = []
    for (tech, step), g in df.groupby(["technique", "label"]):
        df_non_capped = g[~g["capped"]]
        num_rows = len(df_non_capped)
        rows.append({
            "technique": tech, "label": step, "num_systems": num_rows, "num_sig_holm": int(df_non_capped["sig_holm"].sum()) if num_rows else 0,
            "num_sig_bh": int(df_non_capped["sig_bh"].sum()) if num_rows else 0, "median_p": float(df_non_capped["p"].median()) if num_rows else np.nan,
            "median_diff": float(df_non_capped["median_diff_pairs"].median()) if num_rows else np.nan,
        })
    new_df = pd.DataFrame(rows)
    new_df["order"] = new_df["label"].map(label_order)
    return new_df.sort_values(["technique", "order"]).drop(columns="order").reset_index(drop=True)

def run_wilcoxon():
    res_data = getRQ2Files()
    df = pairwise_wilcoxon_test(res_data)
    df.to_csv(OUT_PATH / "rq2_prop_comparison.csv", index=False) # type: ignore

    summary_df = summarize_pair_tech(df)
    summary_df.to_csv(OUT_PATH / "rq2_summary_per_tech.csv", index=False)
    with pd.option_context("display.width", 200, "display.max_columns", 20, "display.float_format", "{:.4g}".format):
        print(summary_df.to_string(index=False))

def run():
    OUT_PATH.mkdir(parents=True, exist_ok=True)
    run_spearman()
    run_wilcoxon()

if __name__ == "__main__":
    run()



