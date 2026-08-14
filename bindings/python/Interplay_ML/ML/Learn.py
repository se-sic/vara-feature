import argparse
import numpy as np
import pandas as pd
import json as json
import time as time
import os
import glob
from pathlib import Path
from joblib import Parallel, delayed
from .Metrics import mre
from ..Sampling.Config import GridPaths, TSize, Proportions, Runs, Systems, Results, set_cap
from .MlTechniques import ml_models
from .GetData import getSample, getTrueData, split

def fit(X_S, Y_S, X_E, Y_E, technique, estimator, params, system):
    t_start: float = time.time()
    cap = set_cap(system)

    if cap is not None and technique in ("SVR", "kNN", "KRR") and len(X_S) > cap:
        mult_idx = np.random.RandomState(0).choice(len(X_S), cap, replace=False)
        X_S, Y_S = X_S[mult_idx], Y_S[mult_idx]
    if technique == "kNN":
        default = params.get("n_neighbors", 5)
        params = {**params, "n_neighbors": min(default, max(1, len(X_S)-1))}

    estimator.set_params(**params)
    estimator.fit(X_S, Y_S)
    Y_hat = estimator.predict(X_E)
    final_mre = mre(Y_E, Y_hat)

    return {
        "technique": technique, 
        "mre": final_mre,
        "params": json.dumps(params, default=str),
        "trained_count": len(X_S),
        "test_count": len(X_E),
        "learn_time": round(time.time() - t_start, 2)
    }

def run_one_seed(r, rq, system, strategy, val, size_type, true_data, hyper_params, techniques):
    rows = []
    for ml_tech, (est, _) in techniques.items():
        params = hyper_params[f"{system}/{ml_tech}/rq{rq}/{size_type}{val}"]
        tuned = params != {}
        if rq == 1:
            sample_data = getSample(strategy, system, val, r, rq, None)
        else:
            sample_data = getSample(strategy, system, None, r, rq, val)
        
        X_S, Y_S, X_E, Y_E = split(sample_data, true_data)
        res = fit(X_S, Y_S, X_E, Y_E, ml_tech, est, params, system)
        res.update(system = system, strategy = strategy, rq = rq, size_type = f"{size_type}{val}", seed = r, tuned = tuned)
        rows.append(res)
        print(f"{system} {ml_tech} with {val} done", flush=True)
    print(f"{system} {size_type} with {val} done", flush=True)
    return rows

def run(rq, system, strategy, out_path, only_prop=None):
    hyper_params = json.load(open(GridPaths))
    techniques = ml_models()
    true_data = getTrueData(system)
    rows = []
    its = None

    if strategy in ("twise", "sbs"):
        its = [1]
    else:
        its = range(1, Runs+1)
    for size_type, val in getSize(rq):
        if only_prop is not None and val != only_prop:
            continue
        res = Parallel(-1)(
            delayed(run_one_seed)(r, rq, system, strategy, val, size_type, true_data, hyper_params, techniques)
            for r in its
        )
        for rlist in res:
            rows.extend(rlist) # type: ignore
    Path(out_path).parent.mkdir(parents=True, exist_ok=True)
    pd.DataFrame(rows).to_csv(out_path, index=False)

def getSize(rq):
    if rq == 1:
        for t in TSize:
            yield f"T", t
    else:
        for p in Proportions:
            yield f"Prop", p

def merge_fittings():
    partials = os.path.join(str(Results), "partials")
    for sys in Systems:
        sys_part = sorted(glob.glob(os.path.join(str(partials), f"rq_2_{sys}_random_*.csv")))
        if len(sys_part) != len(Proportions):
            raise Exception(f"Error: a RQ2 fitting was not performed for system {sys}")
        df = pd.concat([pd.read_csv(part) for part in sys_part], ignore_index=True)
        df.to_csv(os.path.join(str(Results), f"rq_2_{sys}_random.csv"), index=False)
        print(f"{sys}: {len(sys_part)} system files; {len(df)} many rows; nulls={df['mre'].isna().sum()}")
    files = glob.glob(os.path.join(str(Results), "rq_*.csv"))
    print(f"Total: {len(files)} files (want 70)")



if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser_2 = parser.add_subparsers(dest="cmd", required=True)

    fit_parser = parser_2.add_parser("fit")

    fit_parser.add_argument("rq", type=int, choices=[1,2])
    fit_parser.add_argument("system")
    fit_parser.add_argument("strategy")
    fit_parser.add_argument("out_path")
    fit_parser.add_argument("--only_prop", type=float, default=None)

    parser_2.add_parser("merge")

    args = parser.parse_args()

    if args.cmd == "fit":
        run(args.rq, args.system, args.strategy, args.out_path, only_prop=args.only_prop)
    elif args.cmd == "merge":
        merge_fittings()