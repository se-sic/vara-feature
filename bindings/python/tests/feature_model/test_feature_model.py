"""
Tests for generated python bindings for FeatureModel.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM
import vara_feature.fm_parsers as fm_parsers

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModel(unittest.TestCase):
    """
    Check of our generated FeatureModel binding correctly work.
    """

    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        with open(TEST_INPUTS_DIR / "example_feature_model.xml",
                  'r') as fm_file:
            parser = fm_parsers.FeatureModelXmlParser(fm_file.read())
            cls.fm = parser.build_feature_model()

    def test_fm_name(self):
        """ Check if the name of the feature model was correctly set. """
        self.assertEqual(self.fm.name.str(), "ABC")

    def test_path(self):
        """ Check if path accessor is correctly mapped. """
        self.assertEqual(self.fm.path, "test/path/to/root/")

    def test_fm_root(self):
        """ Check if the root of the feature model was correctly set. """
        self.assertIsNotNone(self.fm.get_root())
        self.assertEqual(self.fm.get_root().name.str(), "root")

    def test_fm_size(self):
        """ Check if the size of the loaded feature model matches. """
        self.assertEqual(self.fm.size(), 8)

    def test_iter(self):
        """ Check if we can iterate throught the Features. """
        fm_iter = iter(self.fm)
        self.assertEqual(next(fm_iter).name.str(), "root")
        self.assertEqual(next(fm_iter).name.str(), "A")
        self.assertEqual(next(fm_iter).name.str(), "AA")
        self.assertEqual(next(fm_iter).name.str(), "AB")
        self.assertEqual(next(fm_iter).name.str(), "AC")
        self.assertEqual(next(fm_iter).name.str(), "B")
        self.assertEqual(next(fm_iter).name.str(), "N")
        self.assertEqual(next(fm_iter).name.str(), "C")

    def test_iter_children(self):
        """ Checks if we can iterate over a features children. """
        test_feature_root = self.fm.get_feature("root")
        test_feature_a = self.fm.get_feature("A")
        test_feature_b = self.fm.get_feature("B")
        test_feature_c = self.fm.get_feature("C")

        self.assertEqual(set(iter(test_feature_root)),
                         set(test_feature_root.children()))
        self.assertEqual({test_feature_a, test_feature_b, test_feature_c},
                         set(test_feature_root.children()))
        self.assertTrue(test_feature_root.is_child(test_feature_a))
        self.assertTrue(test_feature_root.is_child(test_feature_b))
        self.assertTrue(test_feature_root.is_child(test_feature_c))
