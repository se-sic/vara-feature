"""
Tests for generated python bindings for FeatureModelXmlWriter.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM
import vara_feature.fm_writer as fm_writer
import vara_feature.fm_parsers as fm_parsers

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModel(unittest.TestCase):
    """
    Check of our generated FeatureModelXmlWriter binding correctly works.
    """
    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        with open(TEST_INPUTS_DIR / "simple_example_feature_model.xml",
                  'r') as fm_file:
            parser = fm_parsers.FeatureModelXmlParser(fm_file.read())
            cls.fm = parser.build_feature_model()

    def test_fm_name(self):
        """ Check if the feature model is written correctly. """
        writer = fm_writer.FeatureModelXmlWriter(self.fm)
        output = writer.get_feature_model_as_string()
        with open(TEST_INPUTS_DIR / "simple_example_feature_model.xml",
                  'r') as fm_file:
            expected = fm_file.read()
        self.assertEqual(output, expected)
