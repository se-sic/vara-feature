import pandas as pd
import numpy as np
import json as json
import time as time
from sklearn.model_selection import GridSearchCV 
from .GetData import getSample, getTrueData, split
from .MlTechniques import ml_models, technique_cv
from .Metrics import SCORER_FUNC
from ..Sampling.Config import Systems, Workspace

def grid_tune():
    out = {}
    for sys in Systems:
        true_data = getTrueData(sys)
        sample_data = getSample("random", sys, 2, 1, 1)
        X_S, Y_S, X_E, Y_E = split(sample_data, true_data)
        for technique, (estimator, grid_params) in ml_models().items():
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
                grid_params = {}
            else:
                grid_search = GridSearchCV(estimator=estimator, param_grid=grid_params, scoring=SCORER_FUNC, cv=cv, n_jobs=1)
                grid_search.fit(X_S, Y_S)
                grid_params = grid_search.best_params_
            out[f"{sys}/{technique}"] = grid_params
    json.dump(out, open(Workspace/"bindings/python/Interplay_ML/ML"/"grid_search_params.json", "w"), indent=2, default=str)

if __name__ == "__main__":
    grid_tune()