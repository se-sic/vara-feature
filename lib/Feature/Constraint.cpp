#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

namespace vara::feature {
void BinaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }
void UnaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }
void PrimaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }

Feature *PrimaryConstraint::getFeature() {
  if (std::holds_alternative<Feature *>(FV)) {
    return std::get<Feature *>(FV);
  } else {
    return std::get<std::unique_ptr<Feature>>(FV).get();
  }
}
} // namespace vara::feature
