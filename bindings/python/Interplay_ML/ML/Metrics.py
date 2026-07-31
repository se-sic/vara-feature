import pandas as pd
import numpy as np
from sklearn.metrics import make_scorer

def mre(Y_true, Y_pred):
    Y_true = np.asarray(Y_true, dtype=float)
    Y_pred = np.asarray(Y_pred, dtype=float)
    return float(np.mean(np.abs(Y_true-Y_pred) / Y_true))

SCORER_FUNC = make_scorer(mre, greater_is_better=False)