"""
Tests for generated python bindings for FeatureModelXmlWriter.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM
import vara_feature.fm_writer as fm_writer

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModel(unittest.TestCase):
    """
    Check of our generated FeatureModelXmlWriter binding correctly works.
    """
    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.fm = FM.loadFeatureModel(TEST_INPUTS_DIR /
                                     "simple_example_feature_model.xml")

    def test_fm_name(self):
        """ Check if the feature model is written correctly. """
        writer = fm_writer.FeatureModelXmlWriter(self.fm)
        output = writer.get_feature_model_as_string()
        with open(TEST_INPUTS_DIR / "simple_example_feature_model.xml",
                  'r') as fm_file:
            expected = fm_file.read()
        self.assertEqual(output, expected)
