import os
import unittest
from pathlib import Path
from typing import List

import pandas as pd

from bindings.python.ml.script import data_helper

# Define the directory containing test input files
TEST_INPUTS_DIR = Path(__file__).resolve().parent.parent / 'TEST_INPUTS'


class TestDataHelper(unittest.TestCase):
    """
    Unit tests for the data_helper module functions.
    """

    def test_load_feature_model_and_extract_names(self):
        """
        Test loading a feature model and extracting feature names.
        """
        feature_model_path = TEST_INPUTS_DIR / "simple_example_feature_model.xml"
        feature_model, features, names = data_helper.load_feature_model_and_extract_names(str(feature_model_path))
        expected_names = ['A', 'B', 'C']
        self.assertEqual(names, expected_names, "Feature names do not match expected values.")

    def test_parse_xml_to_csv(self):
        """
        Test parsing an XML file to a CSV file and verify the contents.
        """
        features: List[str] = ['compression', 'compressionLrzip', 'level', 'level1', 'level2', 'level3', 'level4']
        xml_input_path = TEST_INPUTS_DIR / "simple_example_measurements.xml"
        csv_output_path = TEST_INPUTS_DIR / "test-output-parse-xml-to-csv.csv"

        # Parse XML and generate CSV
        df = data_helper.parse_xml_to_csv(str(xml_input_path), str(csv_output_path), features)

        # Define expected DataFrame
        expected_data = [[1, 1, 1, 1, 0, 0, 0, 1.0], [1, 1, 1, 0, 1, 0, 0, 2.0], [1, 1, 1, 0, 0, 1, 0, 3.0]]
        expected_columns = features + ['Performance']
        expected_df = pd.DataFrame(expected_data, columns=expected_columns)

        # Assert that the generated DataFrame matches the expected DataFrame
        pd.testing.assert_frame_equal(df, expected_df)

        # Assert that the CSV file was created
        self.assertTrue(csv_output_path.exists(), f"CSV file '{csv_output_path}' was not created.")

        # Clean up the generated CSV file
        try:
            os.remove(csv_output_path)
        except OSError as e:
            self.fail(f"Failed to remove CSV file '{csv_output_path}': {e}")

    def test_find_performance_for_configurations(self):
        """
        Test matching configurations with measurements and retrieving performance metrics.
        """
        measurements_csv_path = TEST_INPUTS_DIR / "simple_example_measurements.csv"
        sampling_csv_path = TEST_INPUTS_DIR / "simple_example_sample_configurations.csv"

        df_matched = data_helper.find_performance_for_configurations(str(measurements_csv_path), str(sampling_csv_path))

        # Define expected DataFrame
        expected_data = [[1, 1, 1, 1, 0, 0, 0, 1.0], [1, 1, 1, 0, 0, 1, 0, 3.0]]
        features: List[str] = ['compression', 'compressionLrzip', 'level', 'level1', 'level2', 'level3', 'level4']
        expected_columns = features + ['Performance']
        expected_df = pd.DataFrame(expected_data, columns=expected_columns)

        # Assert that the matched DataFrame matches the expected DataFrame
        pd.testing.assert_frame_equal(df_matched, expected_df)
