# variant_generator.py

import csv
import sys
from typing import List
import random
import vara_feature as vf

from configuration import Configuration
from constraint_system import ConstraintSystem

class VariantGenerator:

    def generate_variants(self, feature_model: vf.feature_model, features_to_consider: List[vf.feature.Feature],
                          strategy: str, sample_size: int = sys.maxsize, t: int = 2) -> List[Configuration]:
        if strategy not in ['random', 't-wise', 'distance']:
            raise ValueError("Invalid strategy. Choose from 'random', 't-wise', or 'distance'.")

        if sample_size <= 0:
            raise ValueError("Sample size must be a positive integer.")

        if strategy == 't-wise' and (t <= 1 or t >= len(features_to_consider)):
            raise ValueError("For t-wise sampling, t must be greater than 0 and less than the number of features.")

        constraint_system = ConstraintSystem()
        solver, feature_to_var, var_to_feature = constraint_system.build_base_solver(feature_model)

        configurations = []

        if strategy == 'distance':

            possible_distances = self._get_possible_distances(features_to_consider)
            clauses = []

            while len(configurations) < sample_size and len(possible_distances)>0:

                distance = self._sample_distance(possible_distances)
                solver = constraint_system.reset_solver()

                # exclude generated configurations
                for clause in clauses:
                    solver.add_clause(clause)

                constraint_system.add_distance_constraint(solver, feature_to_var, distance)

                if not solver.solve():
                    possible_distances.remove(distance)

                model = solver.get_model()

                if model:
                    configuration = self._create_configuration_from_model(model, features_to_consider, var_to_feature)

                    if not self.is_duplicate_configuration(configuration, configurations):
                        configurations.append(configuration)
                        clauses.append([-literal for literal in model if abs(literal) in var_to_feature])

        else:
            while solver.solve() and len(configurations) < sample_size:
                model = solver.get_model()
                configuration = self._create_configuration_from_model(model, features_to_consider, var_to_feature)
                if not self.is_duplicate_configuration(configuration, configurations):
                    configurations.append(configuration)
                solver.add_clause([-literal for literal in model if abs(literal) in var_to_feature])

        return configurations

    def _get_possible_distances(self, features_to_consider: List[vf.feature.Feature]) -> List[int]:
        return list(range(1, len(features_to_consider)))

    def _sample_distance(self, possible_distances: List[int]) -> int:
        return random.choice(possible_distances)

    def _create_configuration_from_model(self, model, features_to_consider, var_to_feature) -> Configuration:
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

    def export_configurations_to_csv(self, configurations: List[Configuration], features: List[vf.feature.Feature],
                                     file_path: str):
        feature_names = [feature.name.str() for feature in features if not isinstance(feature, vf.feature.RootFeature)]

        with open(file_path, 'w', newline='') as csvfile:
            csvwriter = csv.writer(csvfile)
            csvwriter.writerow(feature_names)

            for configuration in configurations:
                row = [1 if configuration.get_option_value(feature_name) == True else 0 for feature_name in
                       feature_names]
                csvwriter.writerow(row)

    @staticmethod
    def is_duplicate_configuration(configuration: Configuration, configurations: List[Configuration]) -> bool:
        return any(str(conf) == str(configuration) for conf in configurations)
