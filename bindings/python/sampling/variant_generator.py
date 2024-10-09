import csv
import random
import sys
from typing import Optional

from .configuration import Configuration
from .constraint_system import *


def export_configurations_to_csv(configurations: List[Configuration], features: List[vf.feature.Feature],
                                 file_path: str) -> None:
    feature_names = [feature.name.str() for feature in features if not isinstance(feature, vf.feature.RootFeature)]
    with open(file_path, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(feature_names)
        for configuration in configurations:
            row = [1 if configuration.get_option_value(feature_name) else 0 for feature_name in feature_names]
            csvwriter.writerow(row)


def _create_configuration_from_model(model: List[int], features_to_consider: List[vf.feature.Feature],
                                     var_to_feature: Dict[int, vf.feature.Feature]) -> Configuration:
    binary_options = {}
    for var in model:
        if var > 0:
            feature = var_to_feature.get(var)
            if feature and feature in features_to_consider:
                if isinstance(feature, vf.feature.BinaryFeature):
                    binary_options[feature.name.str()] = True

    configuration = Configuration()
    for name, value in binary_options.items():
        configuration.set_option(name, value)

    return configuration


def _sample_distance(possible_distances: List[int], random_instance: random.Random) -> int:
    return random_instance.choice(possible_distances)


def _get_possible_distances(features_to_consider: List[vf.feature.Feature]) -> List[int]:
    return list(range(1, len(features_to_consider) + 1))  # Including the total count


def _is_duplicate_configuration(configuration: Configuration, configurations: List[Configuration]) -> bool:
    return any(conf == configuration for conf in configurations)


def generate_variants(feature_model: vf.feature_model.FeatureModel, features_to_consider: List[vf.feature.Feature],
                      strategy: str, sample_size: int = sys.maxsize, seed: Optional[int] = None,
                      distances: Optional[List[int]] = None) -> List[Configuration]:
    if strategy not in ['random', 'solver', 'distance', 'diversified-distance']:
        raise ValueError("Invalid strategy. Choose from 'random', 'solver', 'distance' or 'diversified-distance'.")
    if sample_size <= 0:
        raise ValueError("Sample size must be a positive integer.")

    # Initialize random instance with seed for reproducibility
    if seed is not None:
        rand_instance = random.Random(seed)
    else:
        rand_instance = random

    constraint_system = ConstraintSystem()
    solver, feature_to_var, var_to_feature = constraint_system.build_base_solver(feature_model)
    configurations = []

    if 'distance' in strategy:
        clauses = []

        if distances is not None:
            possible_distances = distances
        else:
            possible_distances = _get_possible_distances(features_to_consider)

        if strategy == "diversified-distance":
            candidate_maps = {
                d: {v: 0 for v in var_to_feature if not isinstance(var_to_feature[v], vf.feature.RootFeature)} for d in
                possible_distances}

        while (len(configurations)) < sample_size and possible_distances:
            distance = _sample_distance(possible_distances, rand_instance)

            if strategy == "diversified-distance":
                while candidate_maps[distance]:
                    # Select the least frequently used candidate
                    min_count = min(candidate_maps[distance].values())
                    least_frequent_candidates = [v for v, count in candidate_maps[distance].items() if
                                                 count == min_count]
                    candidate = rand_instance.choice(least_frequent_candidates)

                    solver = constraint_system.reset_solver()
                    solver.append_formula(clauses)
                    add_distance_constraint(solver, feature_to_var, distance, candidate)

                    if solver.solve():
                        break
                    else:
                        candidate_maps[distance].pop(candidate)
                        if not candidate_maps[distance]:
                            possible_distances.remove(distance)
                            break
            else:
                solver = constraint_system.reset_solver()
                solver.append_formula(clauses)
                add_distance_constraint(solver, feature_to_var, distance)

                if not solver.solve():
                    possible_distances.remove(distance)
                    continue

            model = solver.get_model()
            if model:
                configuration = _create_configuration_from_model(model, features_to_consider, var_to_feature)
                if not _is_duplicate_configuration(configuration, configurations):
                    configurations.append(configuration)
                clause = [-literal for literal in model if abs(literal) in var_to_feature]
                clauses.append(clause)

                if strategy == "diversified-distance":
                    for v in clause:
                        if v < 0 and not isinstance(var_to_feature[-v], vf.feature.RootFeature):
                            candidate_maps[distance][-v] += 1

    elif strategy in ['solver', 'random']:
        size = sample_size if strategy == 'solver' else sys.maxsize
        for model in solver.enum_models():
            configuration = _create_configuration_from_model(model, features_to_consider, var_to_feature)
            if not _is_duplicate_configuration(configuration, configurations):
                configurations.append(configuration)
            if len(configurations) >= size:
                break

        if strategy == 'random' and len(configurations) > sample_size:
            configurations = rand_instance.sample(configurations, sample_size)

    solver.delete()

    return configurations
