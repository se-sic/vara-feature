from pathlib import Path 
import pandas as pd
from ..Sampling.Config import Systems, PyStrat, TSize, Results
from ..ML.MlTechniques import ml_models

OUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")

def getResFiles():
    frames = []
    for sys in Systems:
        for strat in PyStrat.keys():
            file = Results/f"rq_1_{sys}_{strat}.csv"
            if not file.exists():
                raise FileNotFoundError(file)
            df = pd.read_csv(file)
            left = {"technique", "mre", "params", "trained_count", "test_count", "learn_time", "system", "strategy", "rq", "size_type", "seed", "tuned"} - set(df.columns)
            if left:
                raise Exception(f"{file} is missing column(s) {left}")
            frames.append(df)
    return pd.concat(frames, ignore_index=True)

def mean_mre(df):
    for(sys, strat), g in df.groupby(["system", "strategy"]):
        if not strat in {"twise", "sbs"}:
            if len(g) != 1800:
                raise Exception(f"{sys}/{strat}: {len(g)} rows, expected 1800")
        elif len(g) != 18:
            raise Exception(f"{sys}/{strat}: {len(g)} rows, expected 1800")
        if g["mre"].isna().any():
            raise Exception(f"{sys}/{strat}: NaN in mre")
    
    val = ["system", "strategy", "technique", "size_type",]
    mre_hat = df.groupby(val)["mre"].mean().rename("mre").reset_index()
    var = df.groupby(val)["mre"].var().rename("var").reset_index()

    var.loc[var["strategy"].isin({"twise","sbs"}), "var"] = 0.0

    for key, val in [("mre", mre_hat), ("var", var)]:
        if len(val) != 1080:
            raise Exception(f"At {key}, {len(val)} many rows weree detected, but 1080 were expected")
    
    return mre_hat, var

def summarize_rows(df, value_col, size_val, det_val):
    size_df = df[df["size_type"] == size_val].copy()
    if not det_val:
        size_df = size_df[~size_df["strategy"].isin({"twise", "sbs"})]
    size_df["comb"] = size_df["strategy"] + "_" + size_df["technique"]
    wide_df = size_df.pivot(index="system", columns="comb", values=value_col)
    strats = PyStrat if det_val else ({key: val for key, val in PyStrat.items() if key not in {"twise", "sbs"}})
    cols = [f"{strat}_{tech}" for strat in strats for tech in ml_models().keys()]

    return wide_df.reindex(index=Systems, columns=cols)

def run():
    OUT_PATH.mkdir(parents=True, exist_ok=True)
    df = getResFiles()
    mre_hat, var = mean_mre(df)
    mre_hat.to_csv(OUT_PATH / "rq1_mre.csv", index=False)
    var.to_csv(OUT_PATH / "rq1_var.csv", index=False)
    for size_t in TSize:
        s_type = f"T{size_t}"
        wide_mre = summarize_rows(mre_hat, "mre", s_type, True)
        wide_var = summarize_rows(var, "var", s_type, False)
        if wide_mre.isna().any().any():
            raise Exception(f"NaN is in pivoted version of mre in size value {s_type}")
        if wide_var.isna().any().any():
            raise Exception(f"NaN is in pivoted version of var in size value {s_type}")
        wide_mre.to_csv(OUT_PATH / f"rq1_wide_mre_{s_type}.csv")
        wide_var.to_csv(OUT_PATH / f"rq1_wide_var_{s_type}.csv")
    print(f"Finished writing mre and var csvs to {OUT_PATH}")

if __name__ == "__main__":
    run()

        
