"""
Tests for generated python bindings for FeatureModel.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM
import vara_feature.xml_parser as xml_p

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModel(unittest.TestCase):
    """
    Check of our generated FeatureModel binding correctly work.
    """
    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        with open(f'{TEST_INPUTS_DIR}/example_feature_model.xml', 'r') as file:
            parser = xml_p.FeatureModelXmlParser(file.read())
            cls.fm = parser.build_feature_model()

    def test_fm_name(self):
        """ Check if the name of the feature model was correctly set. """
        self.assertEqual(self.fm.getName().str(), "ABC")

    def test_fm_root(self):
        """ Check if the root of the feature model was correctly set. """
        self.assertIsNotNone(self.fm.getRoot())
        self.assertEqual(self.fm.getRoot().name, "root")

    def test_fm_size(self):
        """ Check if the size of the loaded feature model matches. """
        self.assertEqual(self.fm.size(), 8)

    def test_iter(self):
        """ Check if we can iterate throught the Features. """
        fm_iter = iter(self.fm)
        self.assertEqual(next(fm_iter).name, "A")
        self.assertEqual(next(fm_iter).name, "AA")
        self.assertEqual(next(fm_iter).name, "AB")
        self.assertEqual(next(fm_iter).name, "root")
        self.assertEqual(next(fm_iter).name, "AC")
        self.assertEqual(next(fm_iter).name, "C")
        self.assertEqual(next(fm_iter).name, "B")
        self.assertEqual(next(fm_iter).name, "N")
