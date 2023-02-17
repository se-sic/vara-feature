"""
Tests for generated python bindings for Constraint classes.
"""
import unittest

import vara_feature.constraint as constraint


class TestConstraintBuilder(unittest.TestCase):
    """ Test ConstraintBuilder functionality.  """

    def test_build(self):
        """ Checks if we can build a constraint. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").implies().feature("B")
        self.assertIsNotNone(cb.build())

    def test_full(self):
        """ Checks if we can build a parenthesized constraint. """
        cb = constraint.ConstraintBuilder()
        cb.left().left().feature("A").implies().left().left().lNot()
        cb.lNot().feature("B").right().implies().left().lNot().left().left()
        cb.lNot().feature("C").right().implies().lNot().lNot().feature("D")
        cb.right().right().right().right().implies().feature("E").right()
        self.assertEqual("((A => (!!B => !(!C => !!D))) => E)", str(cb.build()))
