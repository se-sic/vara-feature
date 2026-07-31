from pathlib import Path

# --------- PATHS -------------------------------------------
Workspace = Path(__file__).parent.parent.parent.parent.parent
Bin = Workspace / "build/bin/random_sampler"
Samples = Workspace / "bindings/python/Interplay_ML/Samples"
Sizes = Samples / "twise_sizes.csv"
SatCounts = Samples / "sat_counts.csv"

System_CSV = Workspace / "bindings/python/Interplay_ML/Data"
# ---------- SYSTEMS -----------------------------------------
Systems = ["7z", 
           "BerkeleyDBC", 
           "Dune", 
           "Hippacc", 
           "Irzip", 
           "JavaGC", 
           "LLVM", 
           "Polly", 
           "VP9", 
           "x264"
]
#---------- STRATEGIES ---------------------------------------
PyStrat = {"random" : "random", "twise": "twise", "sbs": "solver", "dbs": "distance", "ddbs": "diversified-distance"}
#---------- REFERENCE SIZES (AS T) ---------------------------
TSize = [1, 2, 3]
#---------- REFERENCE SIZES (AS PROPOTIIONS) ----------------
Proportions = [0.05, 0.1, 0.3, 0.5]
#---------- REPITITIONS FOR RANDOM STRATEGIES ---------------
Runs = 100