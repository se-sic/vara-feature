import os
import unittest
from pathlib import Path

import pandas as pd

from ...sampling import data_helper

TEST_INPUTS_DIR = Path(os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestDataHelper(unittest.TestCase):

    def test_load_feature_model_and_extract_names(self):
        feature_model, features = data_helper.load_feature_model_and_extract_names(
            TEST_INPUTS_DIR / "simple_example_feature_model.xml")
        self.assertEqual(features, ['A', 'B', 'C'])

    def test_parse_xml_to_csv(self):
        features = ['compression', 'compressionLrzip', 'level', 'level1', 'level2', 'level3', 'level4']
        df = data_helper.parse_xml_to_csv(TEST_INPUTS_DIR / "simple_example_measurements.xml",
                                          'test-output-parse-xml-to-csv.csv', features)

        expected_data = [[1, 1, 1, 1, 0, 0, 0, 1.0], [1, 1, 1, 0, 1, 0, 0, 2.0], [1, 1, 1, 0, 0, 1, 0, 3.0]]
        expected_df = pd.DataFrame(expected_data, columns=features + ['Performance'])
        pd.testing.assert_frame_equal(df, expected_df)
        self.assertTrue(os.path.exists('test-output-parse-xml-to-csv.csv'))
        os.remove('test-output-parse-xml-to-csv.csv')

    def test_find_performance_for_configurations(self):
        features = ['compression', 'compressionLrzip', 'level', 'level1', 'level2', 'level3', 'level4']
        df = data_helper.find_performance_for_configurations(TEST_INPUTS_DIR / "simple_example_measurements.csv",
                                                             TEST_INPUTS_DIR / "simple_example_sample_configurations.csv")
        expected_data = [[1, 1, 1, 1, 0, 0, 0, 1.0], [1, 1, 1, 0, 0, 1, 0, 3.0]]
        expected_df = pd.DataFrame(expected_data, columns=features + ['Performance'])
        pd.testing.assert_frame_equal(df, expected_df)
