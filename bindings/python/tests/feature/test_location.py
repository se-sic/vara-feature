"""
Tests for generated python bindings for Location classes.
"""
import unittest

from vara_feature.feature import Location, LineColumnOffset


class TestLineColumnOffset(unittest.TestCase):
    """ Test LineColumnOffset bindings. """
    def test_line_number(self):
        """ Checks if line numbers are correctly mapped. """
        lc_offset_0 = LineColumnOffset(3, 4)
        lc_offset_1 = LineColumnOffset(13, 20)

        self.assertEqual(lc_offset_0.line_number, 3)
        self.assertEqual(lc_offset_1.line_number, 13)

    def test_column_offset_accessor(self):
        """ Checks if the column offset is correctly mapped. """
        lc_offset_0 = LineColumnOffset(3, 4)
        lc_offset_1 = LineColumnOffset(13, 20)

        self.assertEqual(lc_offset_0.column_offset, 4)
        self.assertEqual(lc_offset_1.column_offset, 20)
    
    def test_line_column_offset_setter(self):
        """ Checks if the column offset is correctly mapped. """
        lc_offset_0 = LineColumnOffset(3, 4)
        lc_offset_0.line_number = 4
        lc_offset_0.column_offset = 5

        self.assertEqual(lc_offset_0.line_number, 4)
        self.assertEqual(lc_offset_0.column_offset, 5)


class TestLocation(unittest.TestCase):
    """ Test Location bindings.  """
    def test_location_construction(self):
        """ Checks if we can correclty construct different Locations. """
        path_0 = "foo/bar/bazz"
        path_1 = "foo/bar/buzz"
        path_2 = "foo/bar/bozz"
        start_lc_offset = LineColumnOffset(3, 4)
        end_lc_offset = LineColumnOffset(4, 20)

        loc_0 = Location(path_0)
        loc_1 = Location(path_1, start_lc_offset)
        loc_2 = Location(path_2, start_lc_offset, end_lc_offset)

        self.assertEqual(loc_0.path, path_0)

        self.assertEqual(loc_1.path, path_1)
        self.assertEqual(loc_1.get_start(), start_lc_offset)

        self.assertEqual(loc_2.path, path_2)
        self.assertEqual(loc_2.get_start(), start_lc_offset)
        self.assertEqual(loc_2.get_end(), end_lc_offset)

    def test_path(self):
        """ Checks if the path accessor is correctly mapped. """

    def test_get_start(self):
        """ Checks if the start accessor is correctly mapped. """
        start_lc_offset = LineColumnOffset(3, 4)
        loc = Location("foo/bar/buzz", start_lc_offset)

        self.assertEqual(loc.get_start(), start_lc_offset)
        self.assertIsNone(loc.get_end())

    def test_get_end(self):
        """ Checks if the end accessor is correctly mapped. """
        start_lc_offset = LineColumnOffset(3, 4)
        end_lc_offset = LineColumnOffset(4, 20)
        loc = Location("foo/bar/buzz", start_lc_offset, end_lc_offset)

        self.assertEqual(loc.get_start(), start_lc_offset)
        self.assertEqual(loc.get_end(), end_lc_offset)
