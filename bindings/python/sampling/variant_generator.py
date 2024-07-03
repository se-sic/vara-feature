# variant_generator.py

import csv
import random
import sys

from configuration import Configuration
from constraint_system import *


def export_configurations_to_csv(configurations: List[Configuration], features: List[vf.feature.Feature],
                                 file_path: str):
    feature_names = [feature.name.str() for feature in features if not isinstance(feature, vf.feature.RootFeature)]

    with open(file_path, 'w', newline='') as csvfile:
        csvwriter = csv.writer(csvfile)
        csvwriter.writerow(feature_names)

        for configuration in configurations:
            row = [1 if configuration.get_option_value(feature_name) else 0 for feature_name in feature_names]
            csvwriter.writerow(row)


def _create_configuration_from_model(model, features_to_consider, var_to_feature) -> Configuration:
    binary_options = {}
    numeric_options = {}

    for var in model:
        if var > 0:
            feature = var_to_feature.get(var)
            if feature and feature in features_to_consider:
                if isinstance(feature, vf.feature.BinaryFeature):
                    binary_options[feature.name.str()] = True

                elif isinstance(feature, vf.feature.NumericFeature):
                    numeric_options[feature.name.str()] = var

    configuration = Configuration()
    for name, value in binary_options.items():
        configuration.set_option(name, value)
    for name, value in numeric_options.items():
        configuration.set_option(name, str(value))

    return configuration


def _sample_distance(possible_distances: List[int]) -> int:
    return random.choice(possible_distances)


def _get_possible_distances(features_to_consider: List[vf.feature.Feature]) -> List[int]:
    return list(range(1, len(features_to_consider)))


def _is_duplicate_configuration(configuration: Configuration, configurations: List[Configuration]) -> bool:
    return any(str(conf) == str(configuration) for conf in configurations)


def generate_variants(feature_model: vf.feature_model, features_to_consider: List[vf.feature.Feature], strategy: str,
                      sample_size: int = sys.maxsize) -> List[Configuration]:
    if strategy not in ['random', 'solver', 'distance', 'diversified-distance']:
        raise ValueError("Invalid strategy. Choose from 'random', 'solver', 'distance' or 'diversified-distance'.")

    if sample_size <= 0:
        raise ValueError("Sample size must be a positive integer.")

    constraint_system = ConstraintSystem()
    solver, feature_to_var, var_to_feature = constraint_system.build_base_solver(feature_model)

    configurations = []

    if 'distance' in strategy:

        possible_distances = _get_possible_distances(features_to_consider)
        if strategy == "diversified-distance":
            candidate_maps = {
                d: {v: 0 for v in var_to_feature if not isinstance(var_to_feature[v], vf.feature.RootFeature)} for d in
                possible_distances}
            # candidate_maps = {d: {2: 0, 3: 0} for d in possible_distances} # for testing purposes
        clauses = []

        while (len(configurations)) < sample_size and possible_distances:
            distance = _sample_distance(possible_distances)
            solver = constraint_system.reset_solver()

            # exclude generated configurations
            for clause in clauses:
                solver.add_clause(clause)

            candidate = min(candidate_maps[distance],
                            key=candidate_maps[distance].get) if strategy == "diversified-distance" else None

            add_distance_constraint(solver, feature_to_var, distance, candidate)
            if not solver.solve():

                if strategy == "diversified-distance":
                    candidate_maps[distance].pop(candidate)
                    if not candidate_maps[distance]:
                        possible_distances.remove(distance)
                else:
                    possible_distances.remove(distance)

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

    elif strategy == 'solver' or strategy == 'random':
        size = sample_size if strategy == 'solver' else sys.maxsize
        while solver.solve() and len(configurations) < size:
            model = solver.get_model()
            configuration = _create_configuration_from_model(model, features_to_consider, var_to_feature)
            if not _is_duplicate_configuration(configuration, configurations):
                configurations.append(configuration)
            solver.add_clause([-literal for literal in model if abs(literal) in var_to_feature])

    solver.delete()

    if strategy == 'random' and len(configurations) > sample_size:
        configurations = random.sample(configurations, sample_size)

    return configurations
