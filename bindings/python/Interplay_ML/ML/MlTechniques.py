import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeRegressor
from sklearn.neighbors import KNeighborsRegressor
from sklearn.kernel_ridge import KernelRidge
from sklearn.linear_model import LinearRegression, Lasso
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
            "n_neighbors": [8, 9, 10, 11, 12, 13, 14, 15],
            "weights": ["uniform", "distance"],
            "p": [1, 2, 3]
        }),
        "KRR": (TransformedTargetRegressor(KernelRidge(), transformer=StandardScaler()), {
            "regressor__alpha": [0.0000001, 0.0001, 0.01, 0.02, 0.05, 0.1],
            "regressor__kernel": ["linear", "rbf", "poly"],
            "regressor__gamma": [0.01, 0.05, 0.1, 0.2],
            "regressor__degree": [1, 2, 3]
        }),
        "MR": (Pipeline([
            ("poly", PolynomialFeatures(degree=2, interaction_only=True, include_bias=False)),
            ("lasso", Lasso(max_iter=10000, tol=0.0003, random_state=RAND))]),
            {"lasso__alpha": [0.001, 0.1, 1.0, 10.0, 100.0, 1000.0]
        }),
        "RF": (RandomForestRegressor(random_state=RAND, n_jobs=1), {
            "n_estimators": [10, 12, 15, 18, 20],
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