import pandas as pd
import glob 
import os 
from bindings.python.Interplay_ML.Sampling.Config import Results, Systems

partials = os.path.join(Results, "partials")
for sys in Systems:
    pars = sorted(glob.glob(os.path.join(partials, f"rq_2_{sys}_random_*.csv")))
    if not pars:
        print(f"no partial files for {sys}"); continue
    df = pd.concat([pd.read_csv(p) for p in pars], ignore_index=True)
    out = os.path.join(Results, f"rq_2_{sys}_random.csv")
    df.to_csv(out, index=False)