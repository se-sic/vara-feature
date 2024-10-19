import unittest
from pathlib import Path

import vara_feature.feature_model as FM

from bindings.python.ml.sampling.constraint_system import *

# Define test inputs directory
TEST_INPUTS_DIR = Path(__file__).resolve().parent.parent / 'TEST_INPUTS'


class TestRelationship(unittest.TestCase):
    """Unit tests for the Relationship class using a real feature model XML."""

    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.feature_model = FM.loadFeatureModel(TEST_INPUTS_DIR / "simple_example_feature_model.xml")
        cls.root_feature = cls.feature_model.get_feature("root")
        cls.child_feature_a = cls.feature_model.get_feature("A")
        cls.child_feature_b = cls.feature_model.get_feature("B")
        cls.child_feature_c = cls.feature_model.get_feature("C")

        # Initialize a Relationship for testing
        cls.relationship = Relationship(cls.root_feature,
            [cls.child_feature_a, cls.child_feature_b, cls.child_feature_c], Relationship.ALTERNATIVE)

    def test_initialization(self):
        """Test initialization of the Relationship."""
        self.assertEqual(self.relationship.parent_feature, self.root_feature)
        self.assertListEqual(self.relationship.child_features,
                             [self.child_feature_a, self.child_feature_b, self.child_feature_c])
        self.assertEqual(self.relationship.relationship_kind, Relationship.ALTERNATIVE)

    def test_string_representation(self):
        """Test the string representation of the Relationship."""
        expected_string = (f"Relationship(kind={Relationship.ALTERNATIVE}, "
                           f"parent={self.root_feature.name.str()}, "
                           f"children={[child.name.str() for child in self.relationship.child_features]})")
        self.assertEqual(str(self.relationship), expected_string)

    def test_add_distance_constraint(self):
        """Test adding a distance constraint to the solver."""
        # Create a mock solver
        solver = Solver(name='Glucose42')
        option_to_var = {self.child_feature_a: 1, self.child_feature_b: 2, self.child_feature_c: 3}

        # Call the method with a specified target distance
        distance = 2
        add_distance_constraint(solver, option_to_var, target_distance=distance)

        # Verify that the solver enforces the distance constraint
        for model in solver.enum_models():
            selected_vars = [var for var in model if 0 < var < 4]
            self.assertEqual(len(selected_vars), distance)


class TestConstraintSystem(unittest.TestCase):
    """Unit tests for the ConstraintSystem class using a real feature model XML."""

    @classmethod
    def setUpClass(cls):
        """Parse and load a FeatureModel for testing."""
        cls.feature_model = FM.loadFeatureModel(TEST_INPUTS_DIR / "example_feature_model.xml")
        cls.constraint_system = ConstraintSystem()

    def test_extract_constraints(self):
        """Test extracting constraints from the feature model."""
        self.constraint_system = ConstraintSystem()
        feature_aa = self.feature_model.get_feature("AA")
        feature_ab = self.feature_model.get_feature("AB")
        feature_ac = self.feature_model.get_feature("AC")
        feature_c = self.feature_model.get_feature("C")

        self.assertEqual(self.feature_model.name.str(), "ABC")
        self.constraint_system.extract_constraints(self.feature_model)

        # Check extracted clauses
        self.assertEqual(len(self.constraint_system.all_clauses), 1)
        expected_clause = [self.constraint_system.id_pool.id("A"), self.constraint_system.id_pool.id("B")]
        self.assertEqual(self.constraint_system.all_clauses[0], expected_clause)

        # Check excluded constraints
        expected_exclusions = {feature_aa: [feature_ac, feature_c], feature_ab: [feature_ac], feature_ac: [feature_ab],
            feature_c: [feature_aa]}
        self.assertEqual(self.constraint_system.excluded_constraints, expected_exclusions)

    def test_parse_constraint_string(self):
        """Test parsing a constraint string into clauses."""
        constraint_str = "A | B | !C"
        expected_clause = [self.constraint_system.id_pool.id("A"), self.constraint_system.id_pool.id("B"),
            -self.constraint_system.id_pool.id("C")]

        parsed_clause = self.constraint_system.parse_constraint_string(constraint_str)
        self.assertEqual(parsed_clause, expected_clause)

    def test_detect_exclude(self):
        """Test detection of exclusion constraints."""
        feature_a = self.feature_model.get_feature("A")
        feature_b = self.feature_model.get_feature("B")
        feature_c = self.feature_model.get_feature("C")

        self.constraint_system.excluded_constraints[feature_a] = [feature_b]

        result = self.constraint_system.detect_exclude(feature_a, feature_b)
        self.assertTrue(result)

        result = self.constraint_system.detect_exclude(feature_a, feature_c)
        self.assertFalse(result)

    def test_detect_alternatives(self):
        """Test detection of alternative relationships."""
        feature_a = self.feature_model.get_feature("A")
        feature_aa = self.feature_model.get_feature("AA")
        feature_ab = self.feature_model.get_feature("AB")
        feature_ac = self.feature_model.get_feature("AC")

        # Setup mutual exclusions to form alternatives
        self.constraint_system.excluded_constraints[feature_aa] = [feature_ab, feature_ac]
        self.constraint_system.excluded_constraints[feature_ab] = [feature_aa, feature_ac]
        self.constraint_system.excluded_constraints[feature_ac] = [feature_aa, feature_ab]

        self.constraint_system.detect_alternatives(feature_a)

        self.assertIn(feature_a, self.constraint_system.relationships)
        relationship = self.constraint_system.relationships[feature_a]
        self.assertEqual(relationship.relationship_kind, Relationship.ALTERNATIVE)
        self.assertListEqual(relationship.child_features, [feature_aa, feature_ab, feature_ac])

    def test_initialize_feature_vars(self):
        """Test initialization of feature-variable mappings."""
        option_to_var: Dict[vf.feature.Feature, int] = {}
        var_to_option: Dict[int, vf.feature.Feature] = {}
        self.constraint_system.initialize_feature_vars(self.feature_model, option_to_var, var_to_option)

        for feature in self.feature_model:
            if isinstance(feature, (vf.feature.BinaryFeature, vf.feature.RootFeature)):
                self.assertIn(feature, option_to_var)
                var = option_to_var[feature]
                self.assertIn(var, var_to_option)
                self.assertEqual(var_to_option[var], feature)

    def test_add_feature(self):
        """Test adding features to the constraint system."""
        feature_a = self.feature_model.get_feature("A")
        feature_b = self.feature_model.get_feature("B")
        feature_root = self.feature_model.get_feature("root")

        # Initialize option_to_var with mappings
        option_to_var = {feature_root: self.constraint_system.id_pool.id("root"),
            feature_a: self.constraint_system.id_pool.id("A"), feature_b: self.constraint_system.id_pool.id("B")}

        # Add features A and B
        self.constraint_system.add_feature(feature_a, option_to_var)
        self.constraint_system.add_feature(feature_b, option_to_var)

        # Expected clauses based on feature relationships
        expected_clauses = [[-option_to_var[feature_a], option_to_var[feature_root]],
            [-option_to_var[feature_root], option_to_var[feature_a]],
            [-option_to_var[feature_b], option_to_var[feature_root]], ]

        for clause in expected_clauses:
            self.assertIn(clause, self.constraint_system.all_clauses)

    def test_add_relationships(self):
        """Test adding relationships to the constraint system."""
        feature_root = self.feature_model.get_feature("A")
        feature_a = self.feature_model.get_feature("AA")
        feature_b = self.feature_model.get_feature("AB")
        feature_c = self.feature_model.get_feature("AC")

        # Assume root feature "A" has children AA, AB, AC with an alternative relationship
        self.constraint_system.relationships[feature_root] = Relationship(feature_root,
            [feature_a, feature_b, feature_c], Relationship.ALTERNATIVE)

        option_to_var = {feature_root: self.constraint_system.id_pool.id("A"),
            feature_a: self.constraint_system.id_pool.id("AA"), feature_b: self.constraint_system.id_pool.id("AB"),
            feature_c: self.constraint_system.id_pool.id("AC")}

        self.constraint_system.add_relationships(option_to_var)

        # Expected clauses for alternative relationships
        expected_clauses = [[-option_to_var[feature_a], option_to_var[feature_root]],
            [-option_to_var[feature_b], option_to_var[feature_root]],
            [-option_to_var[feature_c], option_to_var[feature_root]],
            [-option_to_var[feature_root], option_to_var[feature_a], option_to_var[feature_b],
             option_to_var[feature_c]]]

        for clause in expected_clauses:
            self.assertIn(clause, self.constraint_system.all_clauses)

    def test_convert_constraints_to_clauses(self):
        """Test conversion of constraints to SAT clauses."""
        feature_a = self.feature_model.get_feature("A")
        feature_b = self.feature_model.get_feature("B")
        feature_c = self.feature_model.get_feature("C")

        option_to_var = {feature_a: self.constraint_system.id_pool.id("A"),
            feature_b: self.constraint_system.id_pool.id("B"), feature_c: self.constraint_system.id_pool.id("C")}

        # Setup excluded constraints
        self.constraint_system.excluded_constraints = {feature_a: [feature_b], feature_b: [feature_c]}

        # Setup implied constraints
        self.constraint_system.implied_constraints = {feature_a: [feature_c], feature_c: [feature_a]}

        self.constraint_system.convert_constraints_to_clauses(option_to_var)

        expected_exclusion_clauses = [[-option_to_var[feature_a], -option_to_var[feature_b]],
            [-option_to_var[feature_b], -option_to_var[feature_c]]]

        expected_implication_clauses = [[-option_to_var[feature_a], option_to_var[feature_c]],
            [-option_to_var[feature_c], option_to_var[feature_a]]]

        for clause in expected_exclusion_clauses + expected_implication_clauses:
            self.assertIn(clause, self.constraint_system.all_clauses)

    def test_build_base_solver(self):
        """Test building the base SAT solver."""
        solver, option_to_var, var_to_option = self.constraint_system.build_base_solver(self.feature_model)

        # Verify solver initialization
        self.assertIsInstance(solver, Solver)
        self.assertGreater(solver.nof_clauses(), 0)

        # Verify variable mappings
        for feature, var in option_to_var.items():
            self.assertIn(var, var_to_option)
            self.assertEqual(var_to_option[var], feature)

    def test_reset_solver(self):
        """Test resetting the SAT solver."""
        # Initial solver state
        solver, _, _ = self.constraint_system.build_base_solver(self.feature_model)
        initial_clause_count = solver.nof_clauses()

        # Reset the solver
        new_solver = self.constraint_system.reset_solver()

        # After reset, solver should have the initial number of clauses
        self.assertEqual(new_solver.nof_clauses(), initial_clause_count)

    def test_build_base_solver_satisfiability(self):
        """Test the satisfiability of the base solver."""
        # Load a simple, satisfiable feature model
        simple_fm = FM.loadFeatureModel(TEST_INPUTS_DIR / "simple_example_feature_model.xml")

        solver, _, _ = ConstraintSystem().build_base_solver(simple_fm)
        is_satisfiable = solver.solve()
        self.assertTrue(is_satisfiable)
