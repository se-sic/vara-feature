from ..Sampling.Config import Samples, System_CSV
import pandas as pd
import numpy as np

def getSample(strategy, system, t, seed=None, rq=None, prop=None):
    if rq is not None:
        if rq == 1:
            sys_path = Samples/"RQ1"
            if strategy == "random":
                sys_path = sys_path/"Random"/system/f"T{t}"/f"{system}_{seed}.csv"
            elif strategy == "twise":
                sys_path = Samples/f"T{t}"/f"{system}_TWiseSample.csv"
            else: 
                sys_path = sys_path/strategy.upper()/system/f"T{t}"/f"{system}_seed_{seed}.csv"
            df = pd.read_csv(sys_path)
            df.attrs["system"] = system
            return df 
        else:
            if prop is not None:
                prop_full = int(round(prop * 100))
                sys_path = Samples/"RQ2"/f"{prop_full}%"/system/f"{system}_{seed}.csv"
                df = pd.read_csv(sys_path)
                df.attrs["system"] = system
                return df 
            else:
                raise KeyError("--prop is not defined")
    else:
        sys_path = Samples/f"T{t}"/f"{system}_TWiseSample.csv"
        df = pd.read_csv(sys_path)
        df.attrs["system"] = system
        return df 

def getTrueData(system):
    sys_path = System_CSV/system/f"measurements_{system}.csv"
    if not sys_path.exists():
        sys_path = System_CSV/system/f"measurements-{system}.csv"
        df = pd.read_csv(sys_path)
        df.attrs["system"] = system
        return df 
    else:
        df = pd.read_csv(sys_path)
        df.attrs["system"] = system

        if not (df["Performance"] > 0).all():
            raise KeyError(f"{system} csv has rows with 0-value performance")
        return df

def feature_names(true_data):
    system = true_data.attrs.get("system", "<unknown>")
    if "Performance" not in true_data.columns:
        raise KeyError(f"Missing performance data for {system}")
    feats = [f for f in true_data.columns if f != "Performance"]
    return feats

def split(sample_data, true_data):
    feat_names = feature_names(true_data=true_data)

    for i, c in enumerate(feat_names):
        u = set(true_data[c].unique())
        print(i, c, "mandatory" if u == {1} else ("dead" if u == {0} else "optional"))

    joined_data = sample_data[feat_names].merge(true_data.reset_index().rename(columns={"index":"_gtidx"}), on=feat_names, how="left", validate="one_to_one")
    assert joined_data["Performance"].notna().all(), "sampled config does not exist"
    sample_idx = joined_data["_gtidx"].astype(int).to_numpy()
    split_row = np.zeros(len(true_data), dtype=bool)
    split_row[sample_idx] = True

    X_S = true_data.loc[split_row, feat_names].to_numpy()
    Y_S = true_data.loc[split_row, "Performance"].to_numpy()
    X_E = true_data.loc[~split_row, feat_names].to_numpy()
    Y_E = true_data.loc[~split_row, "Performance"].to_numpy()

    return X_S, Y_S, X_E, Y_E

#if __name__ == "__main__":
    gt = getTrueData("VP9")
    s = getSample("random", "VP9", 3)
    X_S, Y_S, X_E, Y_E = split(s, gt)
    print(len(X_S), len(X_E), len(gt))