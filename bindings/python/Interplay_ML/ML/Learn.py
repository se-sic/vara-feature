import json as json
import time as time
from .Metrics import mre

def fit(X_S, Y_S, X_E, Y_E, technique, estimator, params):
    t_start = time.time()
    if technique == "kNN" and "n_neighbors" in params:
        params = {**params, "n_neighbors": min(params["n_neighbors"], max(1, len(X_S)-1))}
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