from typing import List, Dict, Tuple

import vara_feature as vf
from pysat.card import CardEnc
from pysat.formula import IDPool
from pysat.solvers import Solver


class Relationship:
    """Represents a relationship between a parent feature and its children."""

    ALTERNATIVE = 'alternative'

    def __init__(self, parent: vf.feature.Feature, children: List[vf.feature.Feature], kind: str):
        """
        Initialize a Relationship.

        Args:
            parent (vf.feature.Feature): The parent feature.
            children (List[vf.feature.Feature]): The child features.
            kind (str): The type of relationship (e.g., 'alternative').
        """
        self.parent = parent
        self.children = children
        self.kind = kind

    @property
    def parent_feature(self) -> vf.feature.Feature:
        """Get the parent feature."""
        return self.parent

    @property
    def child_features(self) -> List[vf.feature.Feature]:
        """Get the list of child features."""
        return self.children

    @property
    def relationship_kind(self) -> str:
        """Get the kind of relationship."""
        return self.kind

    def __str__(self) -> str:
        """Return a string representation of the Relationship."""
        parent_name = self.parent.name.str()
        children_names = [child.name.str() for child in self.children]
        return f"Relationship(kind={self.kind}, parent={parent_name}, children={children_names})"


def add_distance_constraint(solver: Solver, option_to_var: Dict[vf.feature.Feature, int], target_distance: int,
                            candidate: int = None) -> None:
    """
    Add a constraint to ensure that the number of selected options equals the target distance and include the candidate.

    Args:
        solver (Solver): The SAT solver instance.
        option_to_var (Dict[vf.feature.Feature, int]): Mapping from features to variable IDs.
        target_distance (int): The desired number of selected features.
        candidate (int, optional): A specific feature variable to include. Defaults to None.
    """
    # Exclude root features from the variables list
    variables = [var for feature, var in option_to_var.items() if
                 not isinstance(feature, vf.feature.RootFeature)]  # <-- Renamed for clarity

    # Create clauses that enforce exactly target_distance variables are true
    distance_clauses = CardEnc.equals(lits=variables, bound=target_distance)  # <-- Renamed for clarity

    solver.append_formula(distance_clauses)

    if candidate:
        solver.add_clause([candidate])


class ConstraintSystem:
    """Manages constraints and relationships within the feature model."""

    def __init__(self, name: str = 'Glucose42'):
        """
        Initialize the ConstraintSystem.

        Args:
            name (str, optional): Name of the SAT solver. Defaults to 'Glucose42'.
        """
        self.id_pool = IDPool()
        self.excluded_constraints: Dict[vf.feature.Feature, List[vf.feature.Feature]] = {}
        self.implied_constraints: Dict[vf.feature.Feature, List[vf.feature.Feature]] = {}
        self.relationships: Dict[vf.feature.Feature, Relationship] = {}
        self.all_clauses: List[List[int]] = []
        self.base_solver: Solver = None
        self.name = name

    def extract_constraints(self, feature_model: vf.feature_model.FeatureModel) -> None:
        """
        Extract constraints from the feature model.

        Args:
            feature_model (vf.feature_model.FeatureModel): The feature model to extract constraints from.
        """
        for constraint in feature_model.booleanConstraints:
            kind = constraint.constraint.get_kind()

            left_feature = feature_model.get_feature(str(constraint.constraint.get_left_operand()))
            right_feature = feature_model.get_feature(str(constraint.constraint.get_right_operand()))

            if kind == vf.constraint.ConstraintKind.CK_EXCLUDES:
                self.excluded_constraints.setdefault(left_feature, []).append(right_feature)

            elif kind == vf.constraint.ConstraintKind.CK_IMPLIES:
                self.implied_constraints.setdefault(left_feature, []).append(right_feature)

            elif kind == vf.constraint.ConstraintKind.CK_OR:
                clause = self.parse_constraint_string(str(constraint.constraint))
                self.all_clauses.append(clause)

    def parse_constraint_string(self, constraint_str: str) -> List[int]:
        """
        Parse a constraint string into a list of clause integers.

        Args:
            constraint_str (str): The constraint string (e.g., "A | B | !C").

        Returns:
            List[int]: A list of integers representing the clause.
        """
        # Remove parentheses and split by OR operator
        operands = constraint_str.replace('(', '').replace(')', '').split('|')
        clause = []

        for operand in operands:
            operand = operand.strip()
            is_negated = operand.startswith('!')
            if is_negated:
                operand = operand[1:]

            feature_id = self.id_pool.id(operand)
            clause.append(-feature_id if is_negated else feature_id)

        return clause

    def detect_exclude(self, feature_a: vf.feature.Feature, feature_b: vf.feature.Feature) -> bool:
        """
        Check if feature_b is excluded by feature_a.

        Args:
            feature_a (vf.feature.Feature): The feature to check exclusions for.
            feature_b (vf.feature.Feature): The feature to check against.

        Returns:
            bool: True if feature_b is excluded by feature_a, False otherwise.
        """
        return feature_b in self.excluded_constraints.get(feature_a, [])

    def detect_alternatives(self, feature_node: vf.feature.FeatureTreeNode) -> None:
        """
        Detect alternative relationships among the children of a feature node.

        Args:
            feature_node (vf.feature.FeatureTreeNode): The feature node to analyze.
        """
        parent = feature_node.parent()
        children = list(feature_node.children())

        if len(children) > 1 and all(not child.is_optional() and all(
                self.detect_exclude(child, other) and self.detect_exclude(other, child) for other in children if
                child != other) for child in children):
            self.relationships[feature_node] = Relationship(parent, children, Relationship.ALTERNATIVE)

    def initialize_feature_vars(self, feature_model: vf.feature_model.FeatureModel,
                                option_to_var: Dict[vf.feature.Feature, int],
                                var_to_option: Dict[int, vf.feature.Feature]) -> None:
        """
        Initialize mappings between features and SAT variables.

        Args:
            feature_model (vf.feature_model.FeatureModel): The feature model.
            option_to_var (Dict[vf.feature.Feature, int]): Mapping from features to variable IDs.
            var_to_option (Dict[int, vf.feature.Feature]): Mapping from variable IDs to features.
        """
        for feature in feature_model:
            if isinstance(feature, (vf.feature.BinaryFeature, vf.feature.RootFeature)):
                var = self.id_pool.id(feature.name.str())
                option_to_var[feature] = var
                var_to_option[var] = feature

    def add_feature(self, feature: vf.feature.Feature, option_to_var: Dict[vf.feature.Feature, int]) -> None:
        """
        Add the constraints between a feature and its parent to the clauses.

        Args:
            feature (vf.feature.Feature): The feature to add.
            option_to_var (Dict[vf.feature.Feature, int]): Mapping from features to variable IDs.
        """
        if isinstance(feature, vf.feature.BinaryFeature):
            var = option_to_var[feature]
            parent = feature.parent()

            if isinstance(parent, vf.feature.Feature):
                parent_var = option_to_var[parent]
                self.all_clauses.append([-var, parent_var])  # Child implies parent

                if not feature.is_optional():  # For mandatory feature, parent also implies child
                    self.all_clauses.append([-parent_var, var])

            elif isinstance(parent, vf.feature.FeatureTreeNode) and feature not in self.relationships:
                self.detect_alternatives(
                    parent)  # Detect alternative relationships if parent has multiple child features

        elif isinstance(feature, vf.feature.RootFeature):
            self.all_clauses.append([option_to_var[feature]])  # Root feature is selected

    def add_relationships(self, option_to_var: Dict[vf.feature.Feature, int]) -> None:
        """
        Add all detected relationships to the clauses.

        Args:
            option_to_var (Dict[vf.feature.Feature, int]): Mapping from features to variable IDs.
        """
        for relationship in self.relationships.values():
            parent_var = option_to_var[relationship.parent_feature]
            children_vars = [option_to_var[child] for child in relationship.child_features]

            if relationship.relationship_kind == Relationship.ALTERNATIVE:
                for child_var in children_vars:
                    self.all_clauses.append([-child_var, parent_var])  # Child implies parent

                if relationship.parent_feature.is_optional:
                    self.all_clauses.append(
                        [-parent_var] + children_vars)  # Parent selected implies at least one child selected
                else:
                    self.all_clauses.append(children_vars)  # At least one child selected

    def convert_constraints_to_clauses(self, option_to_var: Dict[vf.feature.Feature, int]) -> None:
        """
        Convert all excluded and implied constraints to SAT clauses.

        Args:
            option_to_var (Dict[vf.feature.Feature, int]): Mapping from features to variable IDs.
        """
        # Handle excluded constraints
        for feature, exclusions in self.excluded_constraints.items():
            feature_var = option_to_var[feature]
            for excluded_feature in exclusions:
                excluded_var = option_to_var[excluded_feature]
                self.all_clauses.append([-feature_var, -excluded_var])  # Feature implies not excluded feature

        # Handle implied constraints
        for feature, implications in self.implied_constraints.items():
            feature_var = option_to_var[feature]
            for implied_feature in implications:
                implied_var = option_to_var[implied_feature]
                self.all_clauses.append([-feature_var, implied_var])  # Feature implies implied feature

    def build_base_solver(self, feature_model: vf.feature_model.FeatureModel) -> Tuple[
        Solver, Dict[vf.feature.Feature, int], Dict[int, vf.feature.Feature]]:
        """
        Build and initialize the base SAT solver with all constraints.

        Args:
            feature_model (vf.feature_model.FeatureModel): The feature model.

        Returns:
            Tuple[Solver, Dict[vf.feature.Feature, int], Dict[int, vf.feature.Feature]]:
                The initialized solver, feature-to-variable mapping, and variable-to-feature mapping.
        """
        self.extract_constraints(feature_model)
        solver = Solver(self.name)
        option_to_var: Dict[vf.feature.Feature, int] = {}
        var_to_option: Dict[int, vf.feature.Feature] = {}

        self.initialize_feature_vars(feature_model, option_to_var, var_to_option)

        for feature in feature_model:
            self.add_feature(feature, option_to_var)

        self.add_relationships(option_to_var)
        self.convert_constraints_to_clauses(option_to_var)

        solver.append_formula(self.all_clauses)
        self.base_solver = solver

        return solver, option_to_var, var_to_option

    def reset_solver(self) -> Solver:
        """
        Reset the base solver with all the clauses.

        Returns:
            Solver: The reset solver instance.
        """
        self.base_solver.delete()
        self.base_solver = Solver(self.name, bootstrap_with=self.all_clauses)
        return self.base_solver
