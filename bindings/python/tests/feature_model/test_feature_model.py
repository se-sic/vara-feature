"""
Tests for generated python bindings for FeatureModel.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModel(unittest.TestCase):
    def test_example(self):
        # FM.FeatureModel()
        self.assertTrue(True)
        # TODO: add actual tests
