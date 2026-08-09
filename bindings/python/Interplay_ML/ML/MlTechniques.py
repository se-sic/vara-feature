import pandas as pd
import numpy as np
from sklearn.feature_selection import VarianceThreshold
from sklearn.tree import DecisionTreeRegressor
from sklearn.neighbors import KNeighborsRegressor
from sklearn.kernel_ridge import KernelRidge
from sklearn.linear_model import Lasso
from sklearn.ensemble import RandomForestRegressor
from sklearn.compose import TransformedTargetRegressor
from sklearn.preprocessing import StandardScaler, PolynomialFeatures
from sklearn.pipeline import Pipeline
from sklearn.svm import SVR
from sklearn.model_selection import KFold

RAND = 0

def ml_models():
    return {
        "CART": (DecisionTreeRegressor(random_state=RAND), {
            "splitter": ["best", "random"],
            "max_features": [1.0, "sqrt", "log2"],
            "min_samples_leaf": [1, 2]
        }),
        "kNN": (KNeighborsRegressor(algorithm="auto"), {
            "n_neighbors": [8, 10, 12, 15],
            "weights": ["uniform", "distance"],
            "p": [1, 2]
        }),
        "KRR": (TransformedTargetRegressor(KernelRidge(), transformer=StandardScaler()), {
            "regressor__alpha": [0.0001, 0.01, 0.1],
            "regressor__kernel": ["rbf"],
            "regressor__gamma": [0.01, 0.05, 0.1, 0.2],
        }),
        "MR": (Pipeline([
            ("poly", PolynomialFeatures(degree=2, interaction_only=True, include_bias=False)),
            ("var", VarianceThreshold(0.0)),
            ("lasso", TransformedTargetRegressor(
                Lasso(max_iter=10000, random_state=RAND), transformer=StandardScaler()))]),
            {"lasso__regressor__alpha": [0.001, 0.01, 0.1, 1.0]
        }),
        "RF": (RandomForestRegressor(random_state=RAND, n_jobs=1), {
            "n_estimators": [10, 15, 20],
            "max_features": [1.0, "sqrt", "log2"],
            "min_samples_leaf": [1, 2]
        }),
        "SVR": (TransformedTargetRegressor(SVR(), transformer=StandardScaler()), {
            "regressor__C": [0.5, 1.0, 2.0, 5.0],
            "regressor__epsilon": [0.1, 0.2, 0.3],
            "regressor__coef0": [0.0, 1.0, 2.0],
            "regressor__shrinking": [True, False],
            "regressor__tol": [0.05, 0.1, 0.2, 0.3]
        })
    }

def technique_cv(n):
    if n < 5: 
        return None
    else: 
        return KFold(min(5, n), shuffle=True, random_state=RAND)