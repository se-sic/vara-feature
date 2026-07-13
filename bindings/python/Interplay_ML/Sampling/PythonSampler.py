import os
from pathlib import Path
import vara_feature as vf
import sys
import random
from typing import List, Optional
from ...ml.sampling.configuration import Configuration
from ...ml.script.data_helper import load_feature_model_and_extract_names
from ...ml.sampling.variant_generator import generate_variants, _create_configuration_from_model, _is_duplicate_configuration, export_configurations_to_csv
from ...ml.sampling.constraint_system import ConstraintSystem
from .Path import Samples

PyStrat = {"sbs": "solver", "dbs": "distance", "ddsb": "diversified-distance"}

def generate_randomSolverVariants(feature_model: vf.feature_model.FeatureModel, features_to_consider: List[vf.feature.Feature],
        sample_size: int = sys.maxsize, seed: Optional[int] = None) -> List[Configuration]:
    rand = random.Random(seed)
    solver, feat_to_var, var_to_feat = ConstraintSystem.build_base_solver(feature_model)
    pool = []
    size = sample_size
    for model in solver.enum_models():
        config = _create_configuration_from_model(model, features_to_consider=features_to_consider, var_to_feature=var_to_feat)
        if not _is_duplicate_configuration(configuration=config, configurations=pool):
            pool.append(config)
        if len(pool) >= 20*size:
            break
    configs = rand.sample(pool, min(size, len(pool)))
    solver.delete()
    return configs

def variant_sampler(system_path, strategy, sample_size, seed):
    valid_strategies = ["rbs", "sbs", "rsbs","dbs", "ddbs"]
    fm, feats, _ = load_feature_model_and_extract_names(system_path)
    sampled_configurations = None

    if strategy not in valid_strategies: 
        raise Exception(f"Chosen strategy {strategy} is not valid, please choose a valid one.")
    
    if strategy == "rsbs":
        sampled_configurations = generate_randomSolverVariants(feature_model=fm, features_to_consider=feats, sample_size=sample_size, seed=seed, distances=None)
    else: 
        sampled_configurations = generate_variants(feature_model=fm, features_to_consider=feats, strategy=PyStrat[strategy], sample_size=sample_size, seed=seed, distances=None)
    
    systemPath = Path(system_path).stem
    seedPath = "" if strategy == "sbs" else f"_seed_{seed}"
    stratPath = Samples/strategy.upper()
    sampled_csv_path = stratPath/f"{systemPath}{seedPath}.csv"
    export_configurations_to_csv(configurations=sampled_configurations, features=feats, file_path=sampled_csv_path)