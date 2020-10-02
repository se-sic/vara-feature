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

    def test_iter_children(self):
        """ Checks if we can iterate over a features children. """
        test_feature_1 = feature.BinaryFeature("a", False)
        test_feature_2 = feature.BinaryFeature("b", True)

        root_feature = feature.BinaryFeature("root", False,
                                             feature.Location(""), None,
                                             {test_feature_1, test_feature_2})

        self.assertEqual(set(iter(root_feature)),
                         set(root_feature.children()))
        self.assertEqual({test_feature_1, test_feature_2},
                         set(root_feature.children()))
        self.assertTrue(root_feature.is_child(test_feature_1))

    def test_iter_excludes(self):
        """ Checks if we can iterate over a features children. """
        test_feature_1 = feature.BinaryFeature("a", False)
        test_feature_2 = feature.BinaryFeature("b", True)

        root_feature = feature.BinaryFeature("root", False,
                                             feature.Location(""), None,
                                             set(),
                                             {test_feature_1, test_feature_2})

        self.assertEqual({test_feature_1, test_feature_2},
                         set(root_feature.excludes()))
        self.assertTrue(root_feature.is_excluded(test_feature_1))

    def test_iter_implications(self):
        """ Checks if we can iterate over a features children. """
        test_feature_1 = feature.BinaryFeature("a", False)
        test_feature_2 = feature.BinaryFeature("b", True)

        root_feature = feature.BinaryFeature("root", False,
                                             feature.Location(""), None,
                                             set(), set(),
                                             {test_feature_1, test_feature_2})

        self.assertEqual({test_feature_1, test_feature_2},
                         set(root_feature.implications()))
        self.assertTrue(root_feature.is_implied_by(test_feature_1))

    def test_parent(self):
        """ Checks if we can iterate over a features children. """
        test_feature_1 = feature.BinaryFeature("a", False)
        test_feature_2 = feature.BinaryFeature("aa", True, feature.Location(""),
                                               test_feature_1)

        self.assertFalse(test_feature_2.is_root())
        self.assertEqual(test_feature_1,
                         test_feature_2.parent())
        self.assertTrue(test_feature_2.is_parent(test_feature_1))

    def test_feature_equal(self):
        """ Checks if Feature equality comparison operator is correctly
        mapped."""
        root_feature = feature.BinaryFeature("root", False)
        test_feature_0 = feature.BinaryFeature("Test", False)
        test_feature_1 = feature.BinaryFeature("Test", False)

        self.assertTrue(test_feature_0 == test_feature_1)
        self.assertFalse(test_feature_0 == root_feature)

    def test_feature_location_accessors(self):
        """ Checks if a feature's location is accessible."""
        path = "test"
        start_lco = feature.LineColumnOffset(3, 4)
        end_lco = feature.LineColumnOffset(3, 20)
        loc = feature.Location(path, start_lco, end_lco)
        test_feature_0 = feature.BinaryFeature("Test", False, loc)

        self.assertTrue(test_feature_0.location)
        self.assertEqual(test_feature_0.location.path, path)
        self.assertEqual(test_feature_0.location.start.line_number,
                         3)
        self.assertEqual(
            test_feature_0.location.start.column_offset, 4)
        self.assertEqual(test_feature_0.location.end.line_number, 3)
        self.assertEqual(test_feature_0.location.end.column_offset,
                         20)

    def test_feature_location_setters(self):
        """ Checks if a feature's location is mutable."""
        path = "test"
        start_lco = feature.LineColumnOffset(3, 4)
        end_lco = feature.LineColumnOffset(3, 20)
        loc = feature.Location(path, start_lco, end_lco)
        test_feature_0 = feature.BinaryFeature("Test", False, loc)

        start = test_feature_0.location.start
        end = test_feature_0.location.end
        start.line_number = 4
        end.line_number = 4
        start.column_offset = 2
        end.column_offset = 18

        self.assertTrue(test_feature_0.location)
        self.assertEqual(test_feature_0.location.path, path)
        self.assertEqual(test_feature_0.location.start.line_number,
                         4)
        self.assertEqual(
            test_feature_0.location.start.column_offset, 2)
        self.assertEqual(test_feature_0.location.end.line_number, 4)
        self.assertEqual(test_feature_0.location.end.column_offset,
                         18)

    def test_feature_location_self_assign(self):
        """ Checks if a feature's location is mutable."""
        path = "test"
        start_lco = feature.LineColumnOffset(4, 2)
        end_lco = feature.LineColumnOffset(4, 18)
        loc = feature.Location(path, start_lco, end_lco)
        test_feature_0 = feature.BinaryFeature("Test", False, loc)

        test_feature_0.location = test_feature_0.location
        self.assertTrue(test_feature_0.location)
        self.assertEqual(test_feature_0.location.path, path)
        self.assertEqual(test_feature_0.location.start.line_number,
                         4)
        self.assertEqual(
            test_feature_0.location.start.column_offset, 2)
        self.assertEqual(test_feature_0.location.end.line_number, 4)
        self.assertEqual(test_feature_0.location.end.column_offset,
                         18)


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
        """ Checks if we can successfully create a numeric features.  """
        test_feature = feature.NumericFeature("Foo", (40, 42), False)
        self.assertEqual("Foo", test_feature.name.str())
        self.assertFalse(test_feature.is_optional())

        test_feature = feature.NumericFeature("Foo", [40, 41, 42], False)
        self.assertEqual("Foo", test_feature.name.str())
        self.assertFalse(test_feature.is_optional())
