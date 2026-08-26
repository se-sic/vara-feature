from pathlib import Path 
import pandas as pd
from ..Sampling.Config import Systems, Proportions, Results
from ..ML.MlTechniques import ml_models

OUT_PATH = Path("bindings/python/Interplay_ML/Stats/Out")
def prop_to_str(prop):
    return f"Prop{prop:g}"
Prop_Val_To_Num = {prop_to_str(prop): prop for prop in Proportions}

def is_capped(sys, tech, prop):
    return sys in {"VP9", "JavaGC"} and tech in {"kNN", "KRR", "SVR"} and prop in {"Prop0.15", "Prop0.2"}

def remove_capped(df):
    capped_df = (df["system"].isin({"VP9", "JavaGC"})
                & df["technique"].isin({"kNN", "KRR", "SVR"})
                & df["size_type"].isin({"Prop0.15", "Prop0.2"})
                )
    num_removed = capped_df.sum()
    if num_removed != 1200:
        raise Exception(F"Expected to remove 1200 rows, instead only removed {num_removed}")
    return df[~capped_df].copy()

def getResFiles():
    frames = []
    for sys in Systems:
        file = Results/f"rq_2_{sys}_random.csv"
        if not file.exists():
            raise FileNotFoundError(file)
        df = pd.read_csv(file)
        left = {"technique", "mre", "params", "trained_count", "test_count", "learn_time", "system", "strategy", "rq", "size_type", "seed", "tuned"} - set(df.columns)
        if left:
            raise Exception(f"{file} is missing column(s) {left}")
        frames.append(df)
    return pd.concat(frames, ignore_index=True)

def mean_mre(df):
    for(sys, tech, prop), g in df.groupby(["system", "technique", "size_type"]):
        if is_capped(sys, tech, prop):
            if len(g) != 1800:
                raise Exception(f"Cut/Capped values survived, that should not be happening :(")
        if len(g) != 100:
            raise Exception(f"{sys}/{tech}/{prop}: {len(g)} rows, expected 1800")
        if g["mre"].isna().any():
            raise Exception(f"{sys}/{tech}/{prop}: NaN in mre")
    
    val = ["system", "technique", "size_type"]
    mre_hat = df.groupby(val)["mre"].mean().rename("mre").reset_index()
    var = df.groupby(val)["mre"].var().rename("var").reset_index()

    for proxy_df in  (mre_hat, var):
        proxy_df["proportion"] = proxy_df["size_type"].map(Prop_Val_To_Num)

    for key, val in [("mre", mre_hat), ("var", var)]:
        if len(val) != 348:
            raise Exception(f"At {key}, {len(val)} many rows weree detected, but 348 were expected")
    
    return mre_hat, var

def run():
    OUT_PATH.mkdir(parents=True, exist_ok=True)
    df = getResFiles()
    df = remove_capped(df)
    mre_hat, var = mean_mre(df)
    mre_hat.to_csv(OUT_PATH / "rq2_mre.csv", index=False)
    var.to_csv(OUT_PATH / "rq2_var.csv", index=False)
    print(f"Finished writing mre and var csvs to {OUT_PATH}")

if __name__ == "__main__":
    run()

        
