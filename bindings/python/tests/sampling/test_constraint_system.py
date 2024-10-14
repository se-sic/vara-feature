import os
import unittest
from pathlib import Path

import vara_feature.feature_model as FM

from bindings.python.ml.sampling.constraint_system import *

TEST_INPUTS_DIR = Path(os.path.join(Path(os.path.dirname(__file__)).parent, 'TEST_INPUTS'))


class TestConstraintSystem(unittest.TestCase):
    """Tests for the ConstraintSystem class using a real feature model XML."""

    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.fm = FM.loadFeatureModel(TEST_INPUTS_DIR / "example_feature_model.xml")
        cls.cs = ConstraintSystem()

    def test_extract_constraints(self):
        self.cs = ConstraintSystem()
        feature_aa = self.fm.get_feature("AA")
        feature_ab = self.fm.get_feature("AB")
        feature_ac = self.fm.get_feature("AC")
        feature_c = self.fm.get_feature("C")

        self.assertEqual(self.fm.name.str(), "ABC")
        self.cs.extract_constraints(self.fm)

        # extract boolean constraints
        self.assertEqual(len(self.cs.all_clauses), 1)
        expected_output = [self.cs.id_pool.id("A"), self.cs.id_pool.id("B")]
        self.assertEqual(self.cs.all_clauses[0], expected_output)
        self.assertEqual(len(self.cs.excluded_constraints), 4)
        self.assertEqual({feature_aa: [feature_ac, feature_c], feature_ab: [feature_ac], feature_ac: [feature_ab],
                          feature_c: [feature_aa]}, self.cs.excluded_constraints)

    def test_parse_constraint_string(self):
        constraint_str = "A | B | !C"
        expected_output = [self.cs.id_pool.id("A"), self.cs.id_pool.id("B"), -self.cs.id_pool.id("C")]

        parsed_clause = self.cs.parse_constraint_string(constraint_str)
        self.assertEqual(parsed_clause, expected_output)

    def test_detect_exclude(self):
        feature_a = self.fm.get_feature("A")  # adjust to actual feature fetching
        feature_b = self.fm.get_feature("B")  # adjust to actual feature fetching
        self.cs.excluded_constraints[feature_a] = [feature_b]

        result = self.cs.detect_exclude(feature_a, feature_b)
        self.assertTrue(result)

        feature_c = self.fm.get_feature("C")  # adjust to actual feature fetching
        result = self.cs.detect_exclude(feature_a, feature_c)
        self.assertFalse(result)

    def test_detect_alternatives(self):
        feature_a = self.fm.get_feature("A")
        feature_aa = self.fm.get_feature("AA")
        feature_ab = self.fm.get_feature("AB")
        feature_ac = self.fm.get_feature("AC")

        # Setup mutual exclusions to form alternatives
        self.cs.excluded_constraints[feature_aa] = [feature_ab, feature_ac]
        self.cs.excluded_constraints[feature_ab] = [feature_aa, feature_ac]
        self.cs.excluded_constraints[feature_ac] = [feature_aa, feature_ab]

        self.cs.detect_alternatives(feature_a)

        self.assertIn(feature_a, self.cs.relationships)
        relationship = self.cs.relationships[feature_a]
        self.assertEqual(relationship.get_kind(), Relationship.ALTERNATIVE)
        self.assertListEqual(relationship.get_children(), [feature_aa, feature_ab, feature_ac])

    def test_initialize_feature_vars(self):
        option_to_var = {}
        var_to_option = {}
        self.cs.initialize_feature_vars(self.fm, option_to_var, var_to_option)

        for feature in self.fm:
            if isinstance(feature, vf.feature.BinaryFeature) or isinstance(feature, vf.feature.RootFeature):
                self.assertIn(feature, option_to_var)
                var = option_to_var[feature]
                self.assertIn(var, var_to_option)
                self.assertEqual(var_to_option[var], feature)

    def test_add_feature(self):
        feature_a = self.fm.get_feature("A")
        feature_b = self.fm.get_feature("B")
        feature_root = self.fm.get_feature("root")

        # Initialize option_to_var with some mappings
        option_to_var = {feature_root: self.cs.id_pool.id("root"), feature_a: self.cs.id_pool.id("A"),
            feature_b: self.cs.id_pool.id("B")}

        # Add feature A and B
        self.cs.add_feature(feature_a, option_to_var)
        self.cs.add_feature(feature_b, option_to_var)

        # Assertions based on the implementation
        expected_clauses = [[-option_to_var[feature_a], option_to_var[feature_root]],
            [-option_to_var[feature_root], option_to_var[feature_a]],
            [-option_to_var[feature_b], option_to_var[feature_root]], ]
        for clause in expected_clauses:
            self.assertIn(clause, self.cs.all_clauses)

    def test_add_relationships(self):
        feature_root = self.fm.get_feature("A")
        feature_a = self.fm.get_feature("AA")
        feature_b = self.fm.get_feature("AB")
        feature_c = self.fm.get_feature("AC")
        feature_optional = self.fm.get_feature("C")

        # Assume root feature is "A" with children AA, AB, AC
        self.cs.relationships[feature_root] = Relationship(feature_root, [feature_a, feature_b, feature_c],
            Relationship.ALTERNATIVE)

        option_to_var = {feature_root: self.cs.id_pool.id("A"), feature_a: self.cs.id_pool.id("AA"),
            feature_b: self.cs.id_pool.id("AB"), feature_c: self.cs.id_pool.id("AC")}

        self.cs.add_relationships(option_to_var)

        # Expected clauses for alternative relationships
        expected_clauses = [[-option_to_var[feature_a], option_to_var[feature_root]],
            [-option_to_var[feature_b], option_to_var[feature_root]],
            [-option_to_var[feature_c], option_to_var[feature_root]],
            [-option_to_var[feature_root], option_to_var[feature_a], option_to_var[feature_b],
             option_to_var[feature_c]]]

        for clause in expected_clauses:
            self.assertIn(clause, self.cs.all_clauses)

    def test_convert_constraints_to_clauses(self):
        feature_a = self.fm.get_feature("A")
        feature_b = self.fm.get_feature("B")
        feature_c = self.fm.get_feature("C")

        option_to_var = {feature_a: self.cs.id_pool.id("A"), feature_b: self.cs.id_pool.id("B"),
            feature_c: self.cs.id_pool.id("C")}

        # Setup excluded constraints
        self.cs.excluded_constraints = {feature_a: [feature_b], feature_b: [feature_c]}

        # Setup implied constraints
        self.cs.implied_constraints = {feature_a: [feature_c], feature_c: [feature_a]}

        self.cs.convert_constraints_to_clauses(option_to_var)

        expected_exclusion_clauses = [[-option_to_var[feature_a], -option_to_var[feature_b]],
            [-option_to_var[feature_b], -option_to_var[feature_c]]]

        expected_implication_clauses = [[-option_to_var[feature_a], option_to_var[feature_c]],
            [-option_to_var[feature_c], option_to_var[feature_a]]]

        for clause in expected_exclusion_clauses + expected_implication_clauses:
            self.assertIn(clause, self.cs.all_clauses)

    def test_build_base_solver(self):
        solver, option_to_var, var_to_option = self.cs.build_base_solver(self.fm)

        # Check that the solver is initialized and clauses are added
        self.assertIsInstance(solver, Solver)
        self.assertNotEqual(solver.nof_clauses(), 0)

        # Check variable mappings
        for feature, var in option_to_var.items():
            self.assertIn(var, var_to_option)
            self.assertEqual(var_to_option[var], feature)

    def test_reset_solver(self):
        # Initial solver state
        solver, option_to_var, var_to_option = self.cs.build_base_solver(self.fm)
        initial_clauses = solver.nof_clauses()

        # Reset the solver
        new_solver = self.cs.reset_solver()

        # After reset, solver should have the initial clauses
        self.assertEqual(new_solver.nof_clauses(), initial_clauses)

    def test_build_base_solver_satisfiability(self):
        # Assuming the feature model is satisfiable
        fm = FM.loadFeatureModel(TEST_INPUTS_DIR / "simple_example_feature_model.xml")

        solver, option_to_var, var_to_option = ConstraintSystem().build_base_solver(fm)
        satisfiable = solver.solve()
        self.assertTrue(satisfiable)


class TestRelationship(unittest.TestCase):
    """Tests for the Relationship class using a real feature model XML."""

    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.fm = FM.loadFeatureModel(TEST_INPUTS_DIR / "simple_example_feature_model.xml")
        cls.root_feature = cls.fm.get_feature("root")
        cls.child_feature_a = cls.fm.get_feature("A")
        cls.child_feature_b = cls.fm.get_feature("B")
        cls.child_feature_c = cls.fm.get_feature("C")

        # Initialize a Relationship for testing
        cls.relationship = Relationship(cls.root_feature,
                                        [cls.child_feature_a, cls.child_feature_b, cls.child_feature_c],
                                        Relationship.ALTERNATIVE)

    def test_initialization(self):
        self.assertEqual(self.relationship.get_parent(), self.root_feature)
        self.assertListEqual(self.relationship.get_children(),
                             [self.child_feature_a, self.child_feature_b, self.child_feature_c])
        self.assertEqual(self.relationship.get_kind(), Relationship.ALTERNATIVE)

    def test_string_representation(self):
        expected_string = f"Relationship(kind={Relationship.ALTERNATIVE}, parent={self.root_feature.name.str()}, children={[self.child_feature_a.name.str(), self.child_feature_b.name.str(), self.child_feature_c.name.str()]})"
        self.assertEqual(str(self.relationship), expected_string)

    def test_add_distance_constraint(self):
        # Create a mock solver
        solver = Solver(name='cd19')
        option_to_var = {self.child_feature_a: 1, self.child_feature_b: 2, self.child_feature_c: 3}

        # Call the method with a specified target distance
        distance = 2
        add_distance_constraint(solver, option_to_var, target_distance=distance)
        for m in solver.enum_models():
            self.assertEqual(len(list(filter(lambda x: (x > 0 and x < 4), m))), distance)
