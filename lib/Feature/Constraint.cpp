#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

namespace vara::feature {
bool BinaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }

bool UnaryConstraint::accept(ConstraintVisitor &V) { return V.visit(this); }

bool PrimaryIntegerConstraint::accept(ConstraintVisitor &V) {
  return V.visit(this);
}

bool PrimaryFeatureConstraint::accept(ConstraintVisitor &V) {
  return V.visit(this);
}

Feature *PrimaryFeatureConstraint::getFeature() const {
  if (std::holds_alternative<Feature *>(FV)) {
    return std::get<Feature *>(FV);
  }
  return std::get<std::unique_ptr<Feature>>(FV).get();
}

std::unique_ptr<Constraint> PrimaryFeatureConstraint::clone() {
  return std::make_unique<PrimaryFeatureConstraint>(
      std::make_unique<Feature>(this->getFeature()->getName().str()));
}

std::string PrimaryFeatureConstraint::toString() const {
  return llvm::formatv("{0}", getFeature()->getName());
}
} // namespace vara::feature
