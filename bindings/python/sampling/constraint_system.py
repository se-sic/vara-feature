from typing import List, Dict, Tuple

import vara_feature as vf
from pysat.card import CardEnc
from pysat.formula import IDPool
from pysat.solvers import Solver


class Relationship:
    ALTERNATIVE = 'alternative'
    OR = 'or'

    def __init__(self, parent: vf.feature.Feature, children: List[vf.feature.Feature], kind: str):
        self.parent = parent
        self.children = children
        self.kind = kind

    def get_parent(self):
        return self.parent

    def get_children(self):
        return self.children

    def get_kind(self):
        return self.kind

    def __str__(self):
        parent_name = self.parent.name.str()
        children_names = [child.name.str() for child in self.children]
        return f"Relationship(kind={self.kind}, parent={parent_name}, children={children_names})"


def add_distance_constraint(solver: Solver, option_to_var: Dict[vf.feature.Feature, int], target_distance: int,
                            candidate: int = None):
    """Add a constraint to ensure that the number of selected options equals the target distance and include the
    candidate."""
    vars_list = [var for feature, var in option_to_var.items() if not isinstance(feature, vf.feature.RootFeature)]
    clauses = CardEnc.equals(lits=vars_list, bound=target_distance)

    for clause in clauses:
        solver.add_clause(clause)
    if candidate:
        solver.add_clause([candidate])


class ConstraintSystem:
    def __init__(self, name='cd19'):
        self.id_pool = IDPool()
        self.excluded_constraints: Dict[vf.feature.Feature, List[vf.feature.Feature]] = {}
        self.implied_constraints: Dict[vf.feature.Feature, List[vf.feature.Feature]] = {}
        self.relationships: Dict[vf.feature.Feature, Relationship] = {}
        self.all_clauses = []
        self.base_solver = None
        self.name = name

    def extract_constraints(self, fm: vf.feature_model):
        for c in fm.booleanConstraints:
            constraint = c.constraint
            kind = constraint.get_kind()

            if kind == vf.constraint.ConstraintKind.CK_EXCLUDES:
                left_feature = fm.get_feature(str(constraint.get_left_operand()))
                right_feature = fm.get_feature(str(constraint.get_right_operand()))
                if left_feature not in self.excluded_constraints:
                    self.excluded_constraints[left_feature] = []
                self.excluded_constraints[left_feature].append(right_feature)

            elif kind == vf.constraint.ConstraintKind.CK_IMPLIES:
                left_feature = fm.get_feature(str(constraint.get_left_operand()))
                right_feature = fm.get_feature(str(constraint.get_right_operand()))
                if left_feature not in self.implied_constraints:
                    self.implied_constraints[left_feature] = []
                self.implied_constraints[left_feature].append(right_feature)

            elif kind == vf.constraint.ConstraintKind.CK_OR:
                constraint_str = str(constraint)
                clause = self.parse_constraint_string(constraint_str)
                self.all_clauses.append(clause)

    def parse_constraint_string(self, constraint_str: str) -> List[int]:
        """Parses a constraint string into a list of clause integers."""
        constraint_str = constraint_str.replace('(', '').replace(')', '')
        operands = constraint_str.split('|')
        clause = []

        for operand in operands:
            operand = operand.strip()
            negation = operand.startswith('!')
            if negation:
                operand = operand[1:]

            feature_id = self.id_pool.id(operand)
            if negation:
                feature_id = -feature_id

            clause.append(feature_id)

        return clause

    def detect_exclude(self, a: vf.feature.Feature, b: vf.feature.Feature) -> bool:
        return b in self.excluded_constraints.get(a, [])

    def detect_alternatives(self, feature: vf.feature.FeatureTreeNode):
        parent = feature.parent()
        children = list(feature.children())
        if len(children) > 1 and all(not child.is_optional() and all(
                self.detect_exclude(child, other) and self.detect_exclude(other, child) for other in children if
                child != other) for child in children):
            self.relationships[feature] = Relationship(parent, children, Relationship.ALTERNATIVE)

    def initialize_feature_vars(self, fm: vf.feature_model, option_to_var: Dict[vf.feature.Feature, int],
                                var_to_option: Dict[int, vf.feature.Feature]):
        for feature in fm:
            if isinstance(feature, vf.feature.BinaryFeature) or isinstance(feature, vf.feature.RootFeature):
                var = self.id_pool.id(feature.name.str())
                option_to_var[feature] = var
                var_to_option[var] = feature

    def add_feature(self, feature: vf.feature.Feature, option_to_var: Dict[vf.feature.Feature, int]):
        if isinstance(feature, vf.feature.BinaryFeature):
            var = option_to_var[feature]
            parent = feature.parent()

            if isinstance(parent, vf.feature.Feature):
                parent_var = option_to_var[parent]
                self.all_clauses.append([-var, parent_var])
                if not feature.is_optional():
                    self.all_clauses.append([-parent_var, var])

            elif isinstance(parent, vf.feature.FeatureTreeNode) and not feature in self.relationships:
                self.detect_alternatives(parent)

        elif isinstance(feature, vf.feature.RootFeature):
            self.all_clauses.append([option_to_var[feature]])

    def add_relationships(self, option_to_var: Dict[vf.feature.Feature, int]):
        for relationship in self.relationships.values():
            parent = relationship.get_parent()
            parent_var = option_to_var[parent]
            children = [option_to_var[child] for child in relationship.get_children()]

            if relationship.get_kind() == Relationship.ALTERNATIVE:
                for child in children:
                    self.all_clauses.append([-child, parent_var])
                if parent.is_optional:
                    self.all_clauses.append([-parent_var] + children)
                else:
                    self.all_clauses.append(children)
            elif relationship.get_kind() == Relationship.OR:
                self.all_clauses.append([parent_var] + children)
                for child in children:
                    self.all_clauses.append([-child, parent_var])

    def convert_constraints_to_clauses(self, option_to_var: Dict[vf.feature.Feature, int]):
        """Convert all excluded constraints and implied constraints to clauses and add them to all_clauses."""
        for feature, exclusions in self.excluded_constraints.items():
            feature_var = option_to_var[feature]
            for excluded_feature in exclusions:
                excluded_var = option_to_var[excluded_feature]
                self.all_clauses.append([-feature_var, -excluded_var])

        for feature, implications in self.implied_constraints.items():
            feature_var = option_to_var[feature]
            for implied_feature in implications:
                implied_var = option_to_var[implied_feature]
                self.all_clauses.append([-feature_var, implied_var])

    def build_base_solver(self, fm: vf.feature_model) -> Tuple[
        Solver, Dict[vf.feature.Feature, int], Dict[int, vf.feature.Feature]]:
        self.extract_constraints(fm)
        solver = Solver(self.name)
        option_to_var = {}
        var_to_option = {}

        self.initialize_feature_vars(fm, option_to_var, var_to_option)

        for feature in fm:
            self.add_feature(feature, option_to_var)

        self.add_relationships(option_to_var)

        self.convert_constraints_to_clauses(option_to_var)

        solver.append_formula(self.all_clauses)
        self.base_solver = solver

        return solver, option_to_var, var_to_option

    def reset_solver(self) -> Solver:
        self.base_solver.delete()
        self.base_solver = Solver(self.name, bootstrap_with=self.all_clauses)
        return self.base_solver
