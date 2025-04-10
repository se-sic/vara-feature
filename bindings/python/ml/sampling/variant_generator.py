import csv
import random
import sys
from typing import List, Dict, Optional

import pandas as pd
import vara_feature as vf

from .configuration import Configuration
from .constraint_system import ConstraintSystem, add_distance_constraint


def export_configurations_to_csv(configurations: List[Configuration], features: List[vf.feature.Feature],
        file_path: str) -> None:
    """
    Export a list of configurations to a CSV file.

    Args:
        configurations (List[Configuration]): The configurations to export.
        features (List[vf.feature.Feature]): The list of features to include as columns.
        file_path (str): The path to the output CSV file.
    """
    # Extract feature names excluding RootFeature
    feature_names = [feature.name.str() for feature in features if not isinstance(feature, vf.feature.RootFeature)]

    with open(file_path, 'w', newline='') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerow(feature_names)
        for configuration in configurations:
            row = [1 if configuration.get_option_value(feature_name) else 0 for feature_name in feature_names]
            csv_writer.writerow(row)


def _create_configuration_from_model(model: List[int], features_to_consider: List[vf.feature.Feature],
        var_to_feature: Dict[int, vf.feature.Feature]) -> Configuration:
    """
    Create a Configuration object from a SAT solver model.

    Args:
        model (List[int]): The model returned by the SAT solver.
        features_to_consider (List[vf.feature.Feature]): Features to include in the configuration.
        var_to_feature (Dict[int, vf.feature.Feature]): Mapping from variable IDs to features.

    Returns:
        Configuration: The resulting configuration.
    """
    binary_options = {}
    for var in model:
        if var > 0:
            feature = var_to_feature.get(var)
            if feature and feature in features_to_consider:
                if isinstance(feature, vf.feature.BinaryFeature):  # Only consider binary features
                    binary_options[feature.name.str()] = True

    configuration = Configuration()
    for name, value in binary_options.items():
        configuration.set_option(name, value)

    return configuration


def _sample_distance(possible_distances: List[int], random_instance: random.Random) -> int:
    """
    Sample a distance from the list of possible distances.

    Args:
        possible_distances (List[int]): The possible distances to choose from.
        random_instance (random.Random): The random instance for reproducibility.

    Returns:
        int: The sampled distance.
    """
    return random_instance.choice(possible_distances)


def _get_possible_distances(features_to_consider: List[vf.feature.Feature]) -> List[int]:
    """
    Get all possible distances based on the number of features.

    Args:
        features_to_consider (List[vf.feature.Feature]): Features to consider for distance excluding root feature.

    Returns:
        List[int]: A list of possible distances.
    """
    return list(range(1, len(features_to_consider) + 1))  # Including the total count


def _is_duplicate_configuration(configuration: Configuration, configurations: List[Configuration]) -> bool:
    """
    Check if a configuration already exists in the list.

    Args:
        configuration (Configuration): The configuration to check.
        configurations (List[Configuration]): The list of existing configurations.

    Returns:
        bool: True if duplicate, False otherwise.
    """
    return any(conf == configuration for conf in configurations)


def generate_variants(feature_model: vf.feature_model.FeatureModel, features_to_consider: List[vf.feature.Feature],
        strategy: str, sample_size: int = sys.maxsize, seed: Optional[int] = None,
        distances: Optional[List[int]] = None) -> List[Configuration]:
    """
    Generate variants based on the specified strategy.

    Args:
        feature_model (vf.feature_model.FeatureModel): The feature model to generate variants from.
        features_to_consider (List[vf.feature.Feature]): Features to include in the variants.
        strategy (str): The sampling strategy ('random', 'solver', 'distance', 'diversified-distance').
        sample_size (int, optional): The number of variants to generate. Defaults to sys.maxsize.
        seed (Optional[int], optional): Seed for random number generator. Defaults to None.
        distances (Optional[List[int]], optional): Specific distances to sample. Defaults to None.

    Raises:
        ValueError: If an invalid strategy is provided or sample_size is non-positive.

    Returns:
        List[Configuration]: The generated configurations.
    """
    valid_strategies = ['random', 'solver', 'distance', 'diversified-distance']
    if strategy not in valid_strategies:
        raise ValueError(f"Invalid strategy. Choose from {valid_strategies}.")
    if sample_size <= 0:
        raise ValueError("Sample size must be a positive integer.")

    # Initialize random instance with seed for reproducibility
    rand_instance = random.Random(seed) if seed is not None else random

    constraint_system = ConstraintSystem()
    solver, feature_to_var, var_to_feature = constraint_system.build_base_solver(feature_model)
    configurations = []

    if strategy in ['distance', 'diversified-distance']:
        clauses: List[List[int]] = []

        possible_distances = distances if distances is not None else _get_possible_distances(features_to_consider)

        candidate_maps: Dict[int, Dict[int, int]] = {}
        if strategy == "diversified-distance":
            candidate_maps = {
                d: {v: 0 for v in var_to_feature if not isinstance(var_to_feature[v], vf.feature.RootFeature)} for d in
                possible_distances}

        while len(configurations) < sample_size and possible_distances:
            distance = _sample_distance(possible_distances, rand_instance)

            if strategy == "diversified-distance":
                while candidate_maps.get(distance):
                    # Select the least frequently used candidate
                    min_count = min(candidate_maps[distance].values())
                    least_frequent_candidates = [v for v, count in candidate_maps[distance].items() if
                        count == min_count]
                    candidate = rand_instance.choice(least_frequent_candidates)

                    # Reset solver and add current clauses
                    solver = constraint_system.reset_solver()
                    solver.append_formula(clauses)
                    add_distance_constraint(solver, feature_to_var, distance, candidate)

                    if solver.solve():
                        break
                    else:
                        # Remove candidate if no solution found
                        candidate_maps[distance].pop(candidate)
                        if not candidate_maps[distance]:  # Remove distance if no candidates left
                            possible_distances.remove(distance)
                            break
            else:
                # For 'distance' strategy
                solver = constraint_system.reset_solver()
                solver.append_formula(clauses)
                add_distance_constraint(solver, feature_to_var, distance)

                if not solver.solve():
                    possible_distances.remove(distance)  # Remove distance if no solution found
                    continue

            model = solver.get_model()
            if model:
                configuration = _create_configuration_from_model(model, features_to_consider, var_to_feature)
                if not _is_duplicate_configuration(configuration, configurations):
                    configurations.append(configuration)
                # Add a blocking clause to avoid the same model in future iterations
                clause = [-literal for literal in model if abs(literal) in var_to_feature]
                clauses.append(clause)

                if strategy == "diversified-distance":
                    for var in clause:
                        if var < 0 and not isinstance(var_to_feature[-var], vf.feature.RootFeature):
                            candidate_maps[distance][-var] += 1  # Increment count for the selected candidate

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


def sample_from_csv(sample_size: int, seed: Optional[int], measurements_csv: str) -> List[Configuration]:
    """
    Sample configurations from a measurement CSV file.

    Args:
        sample_size (int): The number of configurations to sample.
        seed (Optional[int]): Seed for random number generator. Defaults to None.
        measurements_csv (str): Path to the measurements CSV file.

    Raises:
        ValueError: If sample_size is non-positive.

    Returns:
        List[Configuration]: The sampled configurations.
    """
    if sample_size <= 0:
        raise ValueError("Sample size must be a positive integer.")

    configurations = []

    df_measurements = pd.read_csv(measurements_csv)
    df_measurements = df_measurements.iloc[:, :-1]  # Remove the performance column
    feature_names = df_measurements.columns.tolist()

    # Randomly select sample_size rows from the dataframe
    sampled_df = df_measurements.sample(n=sample_size, random_state=seed)

    for _, row in sampled_df.iterrows():
        configuration = Configuration()
        for feature_name, value in zip(feature_names, row):
            if value in ('1', 1):
                configuration.set_option(feature_name, True)
        configurations.append(configuration)

    return configurations
