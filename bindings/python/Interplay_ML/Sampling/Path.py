from pathlib import Path

Workspace = Path(__file__).parent.parent.parent.parent.parent
Bin = Workspace / "build/bin/random_sampler"
Samples = Workspace / "bindings/python/Interplay_ML/Samples"
Sizes = Samples / "twise_sizes.csv"
SatCounts = Samples / "sat_counts.csv"