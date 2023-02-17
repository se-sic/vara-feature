"""
Tests for generated python bindings for Feature classes.
"""
import os
import unittest
from pathlib import Path

import vara_feature.constraint as constraint

class TestConstraintBuilder(unittest.TestCase):
    """ Test ConstraintBuilder functionality.  """
    def test_build(self):
        """ Checks if we can build a constraint.  """
        constraint_builder = constraint.ConstraintBuilder()
        self.assertEqual("", str(constraint_builder.build()))
