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

    def test_string(self):
        """ Checks output string. """
        test_feature = feature.BinaryFeature("Foo", False, [], "--foo")
        self.assertEqual("--foo", str(test_feature))

    def test_is_optinal(self):
        """ Checks if the Feature is optional.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertFalse(test_feature.is_optional())

    def test_root(self):
        """ Checks if a feature is root. """
        test_feature = feature.RootFeature("a")
        self.assertTrue(test_feature.is_root())

    def test_false_root(self):
        """ Checks if a feature is root if it has no parents. """
        test_feature = feature.BinaryFeature("a", False)
        self.assertFalse(test_feature.is_root())

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
        loc = feature.Location(path, start_lco, end_lco,
                               feature.Location.Category.necessary)
        test_feature_0 = feature.BinaryFeature("Test", False, [loc])

        gotLoc = list(test_feature_0.locations)[0]
        self.assertEqual(gotLoc.path, path)
        self.assertEqual(gotLoc.start.line_number, 3)
        self.assertEqual(gotLoc.start.column_offset, 4)
        self.assertEqual(gotLoc.end.line_number, 3)
        self.assertEqual(gotLoc.end.column_offset, 20)

    def test_feature_location_setters(self):
        """ Checks if a feature's location is mutable."""
        path = "test"
        start_lco = feature.LineColumnOffset(3, 4)
        end_lco = feature.LineColumnOffset(3, 20)
        loc = feature.Location(path, start_lco, end_lco,
                               feature.Location.Category.necessary)
        test_feature_0 = feature.BinaryFeature("Test", False, [loc])

        gotLoc = list(test_feature_0.locations)[0]
        start = gotLoc.start
        end = gotLoc.end
        start.line_number = 4
        end.line_number = 4
        start.column_offset = 2
        end.column_offset = 18

        self.assertEqual(gotLoc.path, path)
        self.assertEqual(gotLoc.start.line_number, 4)
        self.assertEqual(gotLoc.start.column_offset, 2)
        self.assertEqual(gotLoc.end.line_number, 4)
        self.assertEqual(gotLoc.end.column_offset, 18)

    def test_feature_location_update(self):
        """ Checks if a feature's location is mutable."""
        path1 = "test1"
        start_lco1 = feature.LineColumnOffset(4, 2)
        end_lco1 = feature.LineColumnOffset(4, 18)
        loc1 = feature.Location(path1, start_lco1, end_lco1,
                                feature.Location.Category.necessary)
        path2 = "test2"
        start_lco2 = feature.LineColumnOffset(5, 4)
        end_lco2 = feature.LineColumnOffset(5, 20)
        loc2 = feature.Location(path2, start_lco2, end_lco2,
                                feature.Location.Category.necessary)

        test_feature_0 = feature.BinaryFeature("Test", False, [loc1])
        self.assertTrue(test_feature_0.updateLocation(loc1, loc2))
        gotLoc = list(test_feature_0.locations)[0]
        self.assertEqual(gotLoc.path, path2)
        self.assertEqual(gotLoc.start.line_number, 5)
        self.assertEqual(gotLoc.start.column_offset, 4)
        self.assertEqual(gotLoc.end.line_number, 5)
        self.assertEqual(gotLoc.end.column_offset, 20)

    def test_feature_location_remove1(self):
        """ Checks if a feature's location is mutable."""
        path1 = "test1"
        start_lco1 = feature.LineColumnOffset(4, 2)
        end_lco1 = feature.LineColumnOffset(4, 18)
        loc1 = feature.Location(path1, start_lco1, end_lco1,
                                feature.Location.Category.necessary)
        path2 = "test2"
        start_lco2 = feature.LineColumnOffset(5, 4)
        end_lco2 = feature.LineColumnOffset(5, 20)
        loc2 = feature.Location(path2, start_lco2, end_lco2,
                                feature.Location.Category.necessary)

        test_feature_0 = feature.BinaryFeature("Test", False, [loc1, loc2])
        test_feature_0.removeLocation(loc1)
        gotLoc = list(test_feature_0.locations)[0]
        self.assertEqual(gotLoc.path, path2)
        self.assertEqual(gotLoc.start.line_number, 5)
        self.assertEqual(gotLoc.start.column_offset, 4)
        self.assertEqual(gotLoc.end.line_number, 5)
        self.assertEqual(gotLoc.end.column_offset, 20)

    def test_feature_location_remove2(self):
        """ Checks if a feature's location is mutable."""
        path1 = "test1"
        start_lco1 = feature.LineColumnOffset(4, 2)
        end_lco1 = feature.LineColumnOffset(4, 18)
        loc1 = feature.Location(path1, start_lco1, end_lco1,
                                feature.Location.Category.necessary)
        path2 = "test2"
        start_lco2 = feature.LineColumnOffset(5, 4)
        end_lco2 = feature.LineColumnOffset(5, 20)
        loc2 = feature.Location(path2, start_lco2, end_lco2,
                                feature.Location.Category.necessary)

        test_feature_0 = feature.BinaryFeature("Test", False, [loc1, loc2])
        test_feature_0.removeLocation(list(test_feature_0.locations)[0])
        gotLoc = list(test_feature_0.locations)[0]
        self.assertEqual(gotLoc.path, path2)
        self.assertEqual(gotLoc.start.line_number, 5)
        self.assertEqual(gotLoc.start.column_offset, 4)
        self.assertEqual(gotLoc.end.line_number, 5)
        self.assertEqual(gotLoc.end.column_offset, 20)


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
