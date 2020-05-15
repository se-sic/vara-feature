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
        self.assertEqual("Foo", test_feature.name)

    def test_is_optinal(self):
        """ Checks if the Feature is optional.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertFalse(test_feature.is_optional())


class TestBinaryFeature(unittest.TestCase):
    """ Test BinaryFeature functionality.  """
    def test_create_binary_feature(self):
        """ Checks if we can successfully create a binary features.  """
        test_feature = feature.BinaryFeature("Foo", False)
        self.assertEqual("Foo", test_feature.name)
        self.assertFalse(test_feature.is_optional())


class TestNumericFeature(unittest.TestCase):
    """ Test NumericFeature functionality.  """
    def test_create_binary_feature(self):
        """ Checks if we can successfully create a binary features.  """
        test_feature = feature.NumericFeature("Foo", False, (40, 42))
        self.assertEqual("Foo", test_feature.name)
        self.assertFalse(test_feature.is_optional())

        test_feature = feature.NumericFeature("Foo", False, [40, 41, 42])
        self.assertEqual("Foo", test_feature.name)
        self.assertFalse(test_feature.is_optional())
