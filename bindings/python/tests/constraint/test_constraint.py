"""
Tests for generated python bindings for Constraint classes.
"""
import unittest

import vara_feature.constraint as constraint
import vara_feature.feature_model as FM


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


class TestConstraints(unittest.TestCase):
    """ Test Constraints functionality. """

    def test_boolean_constraints(self):
        """ Check if we can build boolean constraints. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").lOr().feature("B")
        c = FM.BooleanConstraint(cb.build())
        self.assertEqual(str(c), "(A | B)")
        self.assertEqual(str(c.constraint.clone()), "(A | B)")

    def test_non_boolean_constraints(self):
        """ Check if we can build non-boolean constraints. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").add().feature("B")
        c = FM.NonBooleanConstraint(cb.build())
        self.assertIsNotNone(c.constraint)
        self.assertEqual(str(c), "(A + B)")
        self.assertEqual(str(c.constraint.clone()), "(A + B)")

    def test_mixed_constraints(self):
        """ Check if we can build mixed constraints. """
        cb = constraint.ConstraintBuilder()
        cb.feature("A").multiply().feature("B").equal().constant(0)
        c = FM.MixedConstraint(cb.build(), FM.Req.ALL, FM.ExprKind.POS)
        self.assertEqual(c.req, FM.Req.ALL)
        self.assertEqual(c.exprKind, FM.ExprKind.POS)
        self.assertEqual(str(c), "((A * B) = 0)")
        self.assertEqual(str(c.constraint.clone()), "((A * B) = 0)")
