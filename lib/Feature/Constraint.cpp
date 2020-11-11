#include "vara/Feature/Constraint.h"

#include <iterator>

namespace vara::feature {
void BinaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }
void UnaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }
void PrimaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }
} // namespace vara::feature
