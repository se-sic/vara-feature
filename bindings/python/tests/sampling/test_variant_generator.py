import csv
import os
import unittest
from pathlib import Path
from typing import List, Dict

import vara_feature as vf
import vara_feature.feature_model as FM

from bindings.python.ml.sampling.configuration import Configuration
from bindings.python.ml.sampling.variant_generator import generate_variants, sample_from_csv

# Define test inputs directory
TEST_INPUTS_DIR = Path(os.path.join(Path(__file__).parent.parent, 'TEST_INPUTS'))


def load_ground_truth_csv(csv_path: Path) -> List[Configuration]:
    """
    Load ground truth all configurations with measurements from a CSV file.

    Args:
        csv_path (Path): Path to the CSV file.

    Returns:
        List[Configuration]: List of Configuration instances representing the ground truth.
    """
    configurations = []
    with open(csv_path, 'r', newline='') as csvfile:
        csv_reader = csv.reader(csvfile)
        headers = next(csv_reader)
        headers = headers[:-1]  # Drop the Performance column

        for row in csv_reader:
            row = row[:-1]  # Drop the Performance column
            config = Configuration()
            for header, value in zip(headers, row):
                if value in ('1', 1):
                    config.set_option(header, True)
            configurations.append(config)
    return configurations


class TestVariantGenerator(unittest.TestCase):
    """Unit tests for the variant_generator module using ground truth CSV."""

    @classmethod
    def setUpClass(cls):
        """Load the feature model and ground truth configurations once for all tests."""
        cls.fm = FM.loadFeatureModel(TEST_INPUTS_DIR / "VP9.xml")
        cls.features_to_consider = [feature for feature in cls.fm if not isinstance(feature, vf.feature.RootFeature)]
        cls.ground_truth = load_ground_truth_csv(TEST_INPUTS_DIR / 'VP9.csv')
        cls.seed = 42  # Fixed seed for deterministic tests

        # Precompute ground truth configurations grouped by distance
        cls.ground_truth_by_distance: Dict[int, List[Configuration]] = {}
        for config in cls.ground_truth:
            distance = sum(1 for feature in cls.features_to_consider if config.get_option_value(feature.name.str()))
            cls.ground_truth_by_distance.setdefault(distance, []).append(config)

    def test_sample_from_csv(self):
        """Test sampling configurations from a CSV file."""
        sample_size = 10
        seed = 7
        input_csv = TEST_INPUTS_DIR / 'VP9.csv'

        # First sampling run
        configurations_run1 = sample_from_csv(sample_size=sample_size, seed=seed, measurements_csv=input_csv)

        # Second sampling run with the same seed
        configurations_run2 = sample_from_csv(sample_size=sample_size, seed=seed, measurements_csv=input_csv)

        # Third sampling run with a different seed
        configurations_run3 = sample_from_csv(sample_size=sample_size, seed=seed + 10, measurements_csv=input_csv)

        variants_run1_str = set(str(conf) for conf in configurations_run1)
        variants_run2_str = set(str(conf) for conf in configurations_run2)
        variants_run3_str = set(str(conf) for conf in configurations_run3)

        ground_truth_set = set(str(conf) for conf in self.ground_truth)
        # Assert that both runs produce identical sets of configurations
        self.assertEqual(variants_run1_str, variants_run2_str,
            "Variants sampled are not deterministic with the same seed.")

        self.assertNotEqual(variants_run1_str, variants_run3_str,
            "Variants sampled are deterministic with a different seed.")

        # Assert that the generated configurations are a subset of the ground truth
        self.assertTrue(ground_truth_set.issuperset(variants_run1_str),
            "Variants sampled are different than ground truth.")

        self.assertTrue(ground_truth_set.issuperset(variants_run3_str),
            "Variants sampled are different than ground truth.")

    def test_distance_sampling(self):
        """
        Test that for each possible distance, the generated configurations using 'distance' strategy
        match the ground truth configurations with the same distance.
        """
        strategy = 'distance'
        # Get all possible distances based on features_to_consider
        possible_distances = self.ground_truth_by_distance.keys()

        for distance in possible_distances:
            with self.subTest(distance=distance):
                # Extract ground truth configurations for this distance
                expected_configs = self.ground_truth_by_distance.get(distance, [])
                expected_set = set(str(conf) for conf in expected_configs)

                # Generate configurations for this distance
                generated_configs = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy,
                    sample_size=len(expected_configs), seed=self.seed, distances=[distance])
                generated_set = set(str(conf) for conf in generated_configs)
                # Compare the generated configurations with the expected ground truth
                self.assertEqual(generated_set, expected_set,
                    f"Configurations for distance {distance} do not match ground truth for '{strategy}' strategy.")

    def test_same_seed(self):
        """
        Test that using the same seed across multiple runs with the same strategy produces identical results.
        Generates 20% samples for each strategy and verifies consistency.
        """
        strategies = ['random', 'solver', 'distance', 'diversified-distance']
        sample_size = max(1, int(0.2 * len(self.ground_truth)))  # Ensure at least one sample

        for strategy in strategies:
            with self.subTest(strategy=strategy):
                # Generate variants for the first run
                variants_first_run = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy, sample_size=sample_size,
                    seed=self.seed)

                # Generate variants for the second run with the same seed
                variants_second_run = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy, sample_size=sample_size,
                    seed=self.seed)

                variants_first_run_str = set(str(conf) for conf in variants_first_run)
                variants_second_run_str = set(str(conf) for conf in variants_second_run)

                # Assert that both runs produce identical sets of configurations
                self.assertEqual(variants_first_run_str, variants_second_run_str,
                    f"Variants generated with strategy '{strategy}' are not deterministic with the same seed.")

    def test_different_seeds(self):
        """
        Test that using different seeds produces different results except for the 'solver' strategy.
        Generates 20% samples with two different seeds and ensures the results differ.
        """
        strategies = ['solver', 'random', 'distance', 'diversified-distance']
        sample_size = max(1, int(0.2 * len(self.ground_truth)))  # Ensure at least one sample

        for strategy in strategies:
            with self.subTest(strategy=strategy):
                variants_seed1 = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy, sample_size=sample_size,
                    seed=self.seed)

                # Generate variants with seed2
                variants_seed2 = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy, sample_size=sample_size,
                    seed=self.seed + 1)
                variants_seed1_str = set(str(conf) for conf in variants_seed1)
                variants_seed2_str = set(str(conf) for conf in variants_seed2)

                if strategy == 'solver':
                    # Assert that the two sets of variants are identical for 'solver' strategy
                    self.assertEqual(variants_seed1_str, variants_seed2_str,
                        f"Variants generated with different seeds should be the same for the '{strategy}' strategy.")
                else:
                    # Assert that the two sets of variants are not identical for other strategies
                    self.assertNotEqual(variants_seed1_str, variants_seed2_str,
                        f"Variants generated with different seeds should differ for the '{strategy}' strategy.")

    def test_all_strategies(self):
        """
        General test to verify that for each strategy, the generated configurations match the ground truth.
        """
        strategies = ['random', 'solver', 'distance', 'diversified-distance']
        sample_size = len(self.ground_truth) + 10  # Include all possible configurations

        for strategy in strategies:
            with self.subTest(strategy=strategy):
                # Generate variants with the fixed seed
                generated_variants = generate_variants(feature_model=self.fm,
                    features_to_consider=self.features_to_consider, strategy=strategy, sample_size=sample_size,
                    seed=self.seed)

                generated_set = set(str(conf) for conf in generated_variants)
                ground_truth_set = set(str(conf) for conf in self.ground_truth)

                self.assertEqual(generated_set, ground_truth_set,
                    f"Generated variants for strategy '{strategy}' differ from ground truth.")
