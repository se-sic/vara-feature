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
        path_3 = "foo/bar/bizz"
        start_lc_offset = LineColumnOffset(3, 4)
        end_lc_offset = LineColumnOffset(4, 20)

        loc_0 = Location(path_0)
        loc_1 = Location(path_1, start_lc_offset)
        loc_2 = Location(path_2, start_lc_offset, end_lc_offset, Location.Category.inessential)
        loc_3 = Location(path_3, start_lc_offset, end_lc_offset)

        self.assertEqual(loc_0.path, path_0)
        self.assertEqual(loc_0.category, Location.Category.necessary)

        self.assertEqual(loc_1.path, path_1)
        self.assertEqual(loc_1.start, start_lc_offset)

        self.assertEqual(loc_2.path, path_2)
        self.assertEqual(loc_2.category, Location.Category.inessential)
        self.assertEqual(loc_2.start, start_lc_offset)
        self.assertEqual(loc_2.end, end_lc_offset)

        self.assertEqual(loc_3.path, path_3)
        self.assertEqual(loc_3.category, Location.Category.necessary)
        self.assertEqual(loc_3.start, start_lc_offset)
        self.assertEqual(loc_3.end, end_lc_offset)

    def test_path(self):
        """ Checks if the path accessor is correctly mapped. """
        path = "foo/bar/buzz"
        loc = Location(path)

        self.assertEqual(loc.path, path)

    def test_category(self):
        """ Checks if the category is correctly mapped. """
        path = "foo/bar/buzz"
        loc = Location(path, None, None, Location.Category.inessential)

        self.assertEqual(loc.category, Location.Category.inessential)
        loc.category = Location.Category.necessary
        self.assertEqual(loc.category, Location.Category.necessary)

    def test_get_start(self):
        """ Checks if the start accessor is correctly mapped. """
        start_lc_offset = LineColumnOffset(3, 4)
        loc = Location("foo/bar/buzz", start_lc_offset)

        self.assertEqual(loc.start, start_lc_offset)
        self.assertIsNone(loc.end)

    def test_get_end(self):
        """ Checks if the end accessor is correctly mapped. """
        start_lc_offset = LineColumnOffset(3, 4)
        end_lc_offset = LineColumnOffset(4, 20)
        loc = Location("foo/bar/buzz", start_lc_offset, end_lc_offset, Location.Category.necessary)

        self.assertEqual(loc.start, start_lc_offset)
        self.assertEqual(loc.end, end_lc_offset)
