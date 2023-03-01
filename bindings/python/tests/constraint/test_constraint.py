"""
Tests for generated python bindings for Constraint classes.
"""
import unittest

import vara_feature.constraint as constraint


class TestConstraintBuilder(unittest.TestCase):
    """ Test ConstraintBuilder functionality. """

    def test_build(self):
        """ Checks if we can build a constraint. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").implies().feature("B")
        self.assertIsNotNone(cb.build())

    def test_call(self):
        """ Checks if we can parenthesize part of a constraint. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").add().feature("B")().multiply().feature("C")
        self.assertEqual("((A + B) * C)", str(cb.build()))

    def test_full(self):
        """ Checks if we can build a parenthesized constraint. """
        cb = constraint.ConstraintBuilder()
        cb.openPar().openPar().feature("A").implies().openPar().openPar().lNot()
        cb.lNot().feature("B").closePar().implies().openPar().lNot().openPar()
        cb.openPar().lNot().feature("C").closePar().implies().lNot().lNot()
        cb.feature("D").closePar().closePar().closePar().closePar().implies()
        cb.feature("E").closePar()
        self.assertEqual("((A => (!!B => !(!C => !!D))) => E)", str(cb.build()))
