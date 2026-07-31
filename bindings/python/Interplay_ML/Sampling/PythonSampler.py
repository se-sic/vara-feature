from pathlib import Path
import vara_feature as vf
import sys
import random
from typing import List, Optional
from ...ml.sampling.configuration import Configuration
from ...ml.script.data_helper import load_feature_model_and_extract_names
from ...ml.sampling.variant_generator import generate_variants, _create_configuration_from_model, _is_duplicate_configuration, export_configurations_to_csv
from ...ml.sampling.constraint_system import ConstraintSystem
from .Config import Samples

PyStrat = {"sbs": "solver", "dbs": "distance", "ddbs": "diversified-distance"}

def generate_randomSolverVariants(feature_model: vf.feature_model.FeatureModel, features_to_consider: List[vf.feature.Feature],
                                  sample_size: int = sys.maxsize, seed: Optional[int] = None) -> List[Configuration]:
    rand = random.Random(seed) if seed is not None else random
    cs = ConstraintSystem()
    # Build an initial solver from the current feature model
    solver, feat_to_var, var_to_feat = cs.build_base_solver(feature_model)
    configurations = []
    constraints = []
    # To realize the randomized solver-based sampler, Henard et al. followed four criteria:
    #    - Changing the order of configuration options for each config draw (1)
    #    - Changing the order of values for each config draw (2)
    #    - Changing the order of constraints for each config draw (3)
    #    - Rebuild entire solver model from scratch at each solver call (4)
    while len(configurations) < sample_size:
        solver = cs.reset_solver()  # (4) solver is reset including all its constraints for each new config draw
        rand.shuffle(constraints) # (3) Already considered constraints are shuffled before appending to the fresh solver 
        solver.append_formula(constraints)
        variables = list(feat_to_var.values())
        rand.shuffle(variables) # (1) the collected variables (VarIDs) are shuffled according to the current seed 
        solver.set_phases([var if rand.random() < 0.5 else -var for var in variables]) # (2) values are set true/false per variable

        if not solver.solve():
            break
        model = solver.get_model()
        if model is None:
            break
        config = _create_configuration_from_model(model, features_to_consider=features_to_consider, var_to_feature=var_to_feat)
        if not _is_duplicate_configuration(configuration=config, configurations=configurations):
            configurations.append(config)
        constraints.append([-literal for literal in model if abs(literal) in var_to_feat]) # To exclude already considered constraints, we flip the literals to make this rule no be considered again
    solver.delete()
    return configurations

def variant_sampler(system_path, strategy, sample_size, t, seed=None):
    valid_strategies = ["sbs","rsbs","dbs", "ddbs"]
    fm, feats, _ = load_feature_model_and_extract_names(system_path)
    sampled_configurations = None

    if strategy not in valid_strategies: 
        raise Exception(f"Chosen strategy {strategy} is not valid, please choose a valid one.")
    
    if strategy == "rsbs":
        sampled_configurations = generate_randomSolverVariants(feature_model=fm, features_to_consider=feats, sample_size=sample_size, seed=seed)
    else: 
        sampled_configurations = generate_variants(feature_model=fm, features_to_consider=feats, strategy=PyStrat[strategy], sample_size=sample_size, seed=seed, distances=None)
    
    systemPath = Path(system_path).stem
    seedPath = "" if strategy == "sbs" else f"_seed_{seed}"
    stratPath = Samples/"RQ1"/strategy.upper()/systemPath/f"T{t}"
    stratPath.mkdir(parents=True, exist_ok=True)
    sampled_csv_path = stratPath/f"{systemPath}{seedPath}.csv"
    export_configurations_to_csv(configurations=sampled_configurations, features=feats, file_path=str(sampled_csv_path))