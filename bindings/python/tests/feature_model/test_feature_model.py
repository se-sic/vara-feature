"""
Tests for generated python bindings for FeatureModel.
"""
import os
import unittest
from pathlib import Path

import vara_feature as vf
import vara_feature.feature_model as FM

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeatureModelStatic(unittest.TestCase):
    """
    Check of our generated FeatureModel bindings work correctly.
    """
    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.fm = FM.loadFeatureModel(TEST_INPUTS_DIR /
                                     "example_feature_model.xml")

    def test_fm_name(self):
        """ Check if the name of the feature model was correctly set. """
        self.assertEqual(self.fm.name.str(), "ABC")

    def test_path(self):
        """ Check if path accessor is correctly mapped. """
        self.assertEqual(self.fm.path, "test/path/to/root/")

    def test_commit(self):
        """ Check if commit accessor is correctly mapped. """
        self.assertEqual(self.fm.commit, "DABAD00")
        commitHash = "DABADAA"
        self.fm.commit = commitHash
        self.assertEqual(self.fm.commit, commitHash)

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

    def test_parent(self):
        """ Checks if we can access a features parent. """
        test_feature_root = self.fm.get_feature("root")
        test_feature_a = self.fm.get_feature("A")
        test_feature_b = self.fm.get_feature("B")

        self.assertFalse(test_feature_a.is_root())
        self.assertFalse(test_feature_b.is_root())
        self.assertEqual(test_feature_root, test_feature_a.parent())
        self.assertEqual(test_feature_a.parent(), test_feature_b.parent())
        self.assertTrue(test_feature_a.is_parent(test_feature_root))
        self.assertTrue(test_feature_b.is_parent(test_feature_root))

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

    def test_merge_models_idempotence(self):
        """ Checks if a feature model merged with itself is unchanged. """
        test_size = self.fm.size()

        test_merged = self.fm.merge_with(self.fm)

        self.assertEqual(test_size, test_merged.size())

        self.assertTrue(test_merged)
        self.assertTrue(test_merged.get_root())
        self.assertTrue(test_merged.get_feature("A"))
        self.assertTrue(test_merged.get_feature("AA"))
        self.assertTrue(test_merged.get_feature("AB"))
        self.assertTrue(test_merged.get_feature("AC"))
        self.assertTrue(test_merged.get_feature("B"))
        self.assertTrue(test_merged.get_feature("C"))
        self.assertTrue(test_merged.get_feature("N"))

    def test_merge_models(self):
        """
        Checks if merging a feature model with another works correctly.

        Merging should unify two models such each feature of both models is present in the resulting model.
        Features should have the same parents as in their source models.
        """
        with open(TEST_INPUTS_DIR / "simple_example_feature_model_merge.xml",
                  'r') as fm_file:
            parser = fm_parsers.FeatureModelXmlParser(fm_file.read())
            test_merge_fm = parser.build_feature_model()
        test_merged = self.fm.merge_with(test_merge_fm)

        self.assertEqual(test_merged.size(), 11)
        self.assertTrue(test_merged.get_root())
        self.assertTrue(test_merged.get_feature("A"))
        self.assertEqual(
            test_merged.get_feature("A").parent().name.str(), "root")
        self.assertTrue(test_merged.get_feature("AA"))
        self.assertEqual(
            test_merged.get_feature("AA").parent().name.str(), "A")
        self.assertTrue(test_merged.get_feature("AB"))
        self.assertEqual(
            test_merged.get_feature("AB").parent().name.str(), "A")
        self.assertTrue(test_merged.get_feature("AC"))
        self.assertEqual(
            test_merged.get_feature("AC").parent().name.str(), "A")
        self.assertTrue(test_merged.get_feature("B"))
        self.assertEqual(
            test_merged.get_feature("B").parent().name.str(), "root")
        self.assertTrue(test_merged.get_feature("C"))
        self.assertEqual(
            test_merged.get_feature("C").parent().name.str(), "root")
        self.assertTrue(test_merged.get_feature("N"))
        self.assertEqual(test_merged.get_feature("N").parent().name.str(), "B")
        self.assertTrue(test_merged.get_feature("X"))
        self.assertEqual(
            test_merged.get_feature("X").parent().name.str(), "root")
        self.assertTrue(test_merged.get_feature("Y"))
        self.assertEqual(
            test_merged.get_feature("Y").parent().name.str(), "root")
        self.assertTrue(test_merged.get_feature("Z"))
        self.assertEqual(
            test_merged.get_feature("Z").parent().name.str(), "root")


class TestFeatureModelModifications(unittest.TestCase):
    """
    Check if our generated FeatureModel bindings work correctly.
    """
    def setUp(cls):
        """Parse and load a FeatureModel for testing."""
        # TODO read model once and copy it for each test
        with open(TEST_INPUTS_DIR / "example_feature_model.xml",
                  'r') as fm_file:
            parser = fm_parsers.FeatureModelXmlParser(fm_file.read())
            cls.fm = parser.build_feature_model()
            cls.assertEqual(cls.fm.size(), 8)

    def test_add_binary_feature(self):
        test_feature_root = self.fm.get_root()
        self.fm.add_binary_feature(test_feature_root, "New", True)
        test_added_feature = self.fm.get_feature("New")
        self.assertTrue(test_added_feature)
        self.assertEqual(test_added_feature.parent(), test_feature_root)
        self.assertEqual(self.fm.size(), 9)

    def test_add_numeric_with_pair_feature(self):
        test_feature_root = self.fm.get_root()
        self.fm.add_numeric_feature(test_feature_root, "New", (0, 10), True)
        test_added_feature = self.fm.get_feature("New")
        self.assertTrue(test_added_feature)
        self.assertEqual(test_added_feature.parent(), test_feature_root)
        self.assertEqual(self.fm.size(), 9)

    def test_add_numeric_with_list_feature(self):
        test_feature_root = self.fm.get_root()
        self.fm.add_numeric_feature(test_feature_root, "New", [0, 1, 2, 3],
                                    True)
        test_added_feature = self.fm.get_feature("New")
        self.assertTrue(test_added_feature)
        self.assertEqual(test_added_feature.parent(), test_feature_root)
        self.assertEqual(self.fm.size(), 9)

    def test_remove_feature_binary(self):
        """ Checks if a binary feature can be removed. """
        test_remove_feature = self.fm.get_feature("C")
        self.fm.remove_feature(test_remove_feature)

        self.assertFalse(self.fm.get_feature("C"))
        self.assertEqual(self.fm.size(), 7)

    def test_remove_feature_numeric(self):
        """ Checks if a numeric feature can be removed. """
        test_remove_feature = self.fm.get_feature("N")
        self.fm.remove_feature(test_remove_feature)

        self.assertFalse(self.fm.get_feature("N"))
        self.assertEqual(self.fm.size(), 7)

    def test_remove_feature_recursive(self):
        """ Checks if a subtree can be removed. """
        test_remove_subroot = self.fm.get_feature("A")
        self.fm.remove_feature(test_remove_subroot, True)

        self.assertFalse(self.fm.get_feature("A"))
        self.assertFalse(self.fm.get_feature("AA"))
        self.assertFalse(self.fm.get_feature("AB"))
        self.assertFalse(self.fm.get_feature("AC"))
        self.assertEqual(self.fm.size(), 4)
