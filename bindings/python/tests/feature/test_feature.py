"""
Tests for generated python bindings for Feature classes.
"""
import os
import unittest
from pathlib import Path

import vara_feature.feature as feature

TEST_INPUTS_DIR = Path(
    os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestFeature(unittest.TestCase):
    """ Test Feature base class functionality.  """
    def test_get_name(self):
        """ Checks if we can access the Features name.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertEqual("Foo", test_feature.name.str())

    def test_is_optinal(self):
        """ Checks if the Feature is optional.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertFalse(test_feature.is_optional())

    def test_is_root(self):
        """ Checks if a feature is root if it has no parents. """
        test_feature = feature.BinaryFeature("root", False)
        self.assertTrue(test_feature.is_root())

    def test_is_not_root(self):
        """ Checks if a feature is not root if it has children. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        root_feature.add_child(test_feature)
        test_feature.add_parent(root_feature)

        self.assertFalse(test_feature.is_root())

    def test_add_child(self):
        """ Checks if a feature is correctly added as a child. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        root_feature.add_child(test_feature)

        self.assertTrue(root_feature.is_child(test_feature))

    def test_iter_children(self):
        """ Checks if we can iterate over a features children. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature_1 = feature.BinaryFeature("Test1", False)
        test_feature_2 = feature.BinaryFeature("Test2", True)

        root_feature.add_child(test_feature_1)
        root_feature.add_child(test_feature_2)

        child_iter = iter(root_feature)
        self.assertEqual(test_feature_1, next(child_iter))
        self.assertEqual(test_feature_2, next(child_iter))

    def test_add_parent(self):
        """ Checks if a feature is correctly added as a parent. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        test_feature.add_parent(root_feature)

        self.assertTrue(test_feature.is_parent(root_feature))

    def test_add_exclude(self):
        """ Checks if a feature is correctly excluded from another. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        test_feature.add_exclude(root_feature)

        self.assertTrue(test_feature.is_excluded(root_feature))

    def test_add_implication(self):
        """ Checks if a feature is correctly can implicate another. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        test_feature.add_implication(root_feature)

        self.assertTrue(test_feature.implicates(root_feature))

    def test_add_alternative(self):
        """ Checks if a feature is correctly added as an alternative from
        another. """
        root_feature = feature.BinaryFeature("root", False)
        test_feature = feature.BinaryFeature("Test", False)

        test_feature.add_alternative(root_feature)

        self.assertTrue(test_feature.is_alternative(root_feature))

    def test_feature_equal(self):
        """ Checks if Feature equality comparison operator is correctly
        mapped."""
        root_feature = feature.BinaryFeature("root", False)
        test_feature_0 = feature.BinaryFeature("Test", False)
        test_feature_1 = feature.BinaryFeature("Test", False)

        self.assertTrue(test_feature_0 == test_feature_1)
        self.assertFalse(test_feature_0 == root_feature)

    def test_feature_less_than(self):
        """ Checks if Feature less than comparison operator is correctly
        mapped."""
        root_feature = feature.BinaryFeature("root", False)
        test_feature_0 = feature.BinaryFeature("Test", False)
        test_feature_1 = feature.BinaryFeature("Test1", False)

        self.assertTrue(root_feature < test_feature_0)
        self.assertTrue(test_feature_0 < test_feature_1)
        self.assertTrue(root_feature < test_feature_1)

    def test_feature_location_accessors(self):
        """ Checks if a feature's location is accessible."""
        root_feature = feature.BinaryFeature("root", False)
        start_lco = feature.LineColumnOffset(3, 4)
        end_lco = feature.LineColumnOffset(3, 20)
        loc = feature.Location("test", start_lco, end_lco)
        test_feature_0 = feature.BinaryFeature("Test", False, loc)



class TestBinaryFeature(unittest.TestCase):
    """ Test BinaryFeature functionality.  """
    def test_create_binary_feature(self):
        """ Checks if we can successfully create a binary features.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertEqual("Foo", test_feature.name.str())
        self.assertFalse(test_feature.is_optional())


class TestNumericFeature(unittest.TestCase):
    """ Test NumericFeature functionality.  """
    def test_create_binary_feature(self):
        """ Checks if we can successfully create a binary features.  """
        test_feature = feature.NumericFeature("Foo", False, (40, 42))
        self.assertEqual("Foo", test_feature.name.str())
        self.assertFalse(test_feature.is_optional())

        test_feature = feature.NumericFeature("Foo", False, [40, 41, 42])
        self.assertEqual("Foo", test_feature.name.str())
        self.assertFalse(test_feature.is_optional())
