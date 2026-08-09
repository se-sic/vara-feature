import glob
import argparse
import pandas as pd
import numpy as np
import json as json
import time as time
from pathlib import Path
from sklearn.model_selection import GridSearchCV, RandomizedSearchCV
from .GetData import getSample, getTrueData, split
from .MlTechniques import ml_models, technique_cv, RAND
from .Metrics import SCORER_FUNC
from ..Sampling.Config import Systems, Workspace, TSize, Proportions, GridPaths, Cap_Default#, set_cap,

Grid_dir_path = GridPaths.parent

def tune_helper(technique, X_S, grid_params, estimator, Y_S, system):
    #cap = set_cap(system)

    if technique in ("SVR", "kNN", "KRR") and len(X_S) > Cap_Default:
        mult_idx = np.random.RandomState(0).choice(len(X_S), Cap_Default, replace=False)
        X_S, Y_S = X_S[mult_idx], Y_S[mult_idx]
    if technique == "kNN":
        lengthSample = len(X_S)
        cv_folds = min(5, lengthSample)
        minFoldTrain = lengthSample - (lengthSample // cv_folds) -1 
        neighbor_params = [k for k in grid_params["n_neighbors"] if k <= minFoldTrain]
        if not neighbor_params:
            neighbor_params = [max(1, minFoldTrain)]
        grid_params = {**grid_params, "n_neighbors" : neighbor_params}
    
    cv = technique_cv(len(X_S))
    if cv is None or not grid_params:
        return {}
    
    if technique in ("KRR", "SVR"):
        search = RandomizedSearchCV(estimator=estimator, param_distributions=grid_params, scoring=SCORER_FUNC, cv=cv, n_jobs=1, random_state=RAND, n_iter=30)
    else:
        search = GridSearchCV(estimator=estimator, param_grid=grid_params, scoring=SCORER_FUNC, cv=cv, n_jobs=1)
    
    search.fit(X_S, Y_S)
    grid_params = search.best_params_
    return grid_params

def grid_tune(system):
    out = {}
    true_data = getTrueData(system)
    print(f"Currently at {system}.", flush=True)

    for t in TSize:
        sample_data = getSample("random", system, t, 0, 1)
        X_S, Y_S, X_E, Y_E = split(sample_data, true_data)
        for technique, (estimator, grid_params) in ml_models().items():
            out[f"{system}/{technique}/rq1/T{t}"] = tune_helper(technique, X_S, grid_params, estimator, Y_S, system)
            print(out[f"{system}/{technique}/rq1/T{t}"], flush=True)

    for p in Proportions:
        sample_data = getSample("random", system, None, 0, 2, p)
        X_S, Y_S, X_E, Y_E = split(sample_data, true_data)
        for technique, (estimator, grid_params) in ml_models().items():
            out[f"{system}/{technique}/rq2/Prop{p}"] = tune_helper(technique, X_S, grid_params, estimator, Y_S, system)
            print(out[f"{system}/{technique}/rq2/Prop{p}"], flush=True)
    
    Grid_dir_path.mkdir(parents=True, exist_ok=True)
    Grid_single = Grid_dir_path/f"{system}.json"
    json.dump(out, open(Grid_single, "w"), indent=2, default=str)
    print(f"{system} is tuned", flush=True)

def merge_grids():
    merged = {}
    grid_singles = sorted(g for g in glob.glob(str(Grid_dir_path / "*.json")) if not g.endswith("grid_search_params.json"))
    if not grid_singles:
        raise FileNotFoundError(f"No grids in {Grid_dir_path}")
    for g in grid_singles:
        merged.update(json.load(open(g)))
    json.dump(merged, open(GridPaths, "w"), indent=2, default=str)

    num_tech = len(ml_models())
    exp = len(Systems)*num_tech*(len(TSize) + len(Proportions))
    if len(merged) != exp:
        print("A system may be missing or a grid may be incomplete.")
    else:
        print(f"Merged {len(grid_singles)} systems")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser_2 = parser.add_subparsers(dest="cmd", required=True)

    sys_parser = parser_2.add_parser("system")
    sys_parser.add_argument("system")

    parser_2.add_parser("merge")

    args = parser.parse_args()

    if args.cmd == "system":
        grid_tune(args.system)
    elif args.cmd == "merge":
        merge_grids()
