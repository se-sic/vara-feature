#ifndef VARA_FEATURE_CONSTRAINT_H
#define VARA_FEATURE_CONSTRAINT_H

#include "vara/Feature/Feature.h"

namespace vara::feature {
//===----------------------------------------------------------------------===//
//                               Constraint
//===----------------------------------------------------------------------===//

/// \brief Tree like representation of constraints between features

class Constraint {
public:
  enum class ConstraintKind { CK_BINARY, CK_UNARY, CK_PRIMARY };

  Constraint(ConstraintKind Kind) : Kind(Kind) {}
  Constraint(const Constraint &) = delete;
  Constraint &operator=(const Constraint &) = delete;
  virtual ~Constraint() = default;

  [[nodiscard]] ConstraintKind getKind() const { return Kind; };

private:
  friend class FeatureModel;
  friend class FeatureModelBuilder;

  ConstraintKind Kind;
};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(string Operator, Constraint *LeftOperand,
                   Constraint *RightOperand)
      : Operator(std::move(Operator)), LeftOperand(LeftOperand),
        RightOperand(RightOperand), Constraint(ConstraintKind::CK_BINARY) {}

private:
  string Operator;
  Constraint *LeftOperand;
  Constraint *RightOperand;
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(string Operator, Constraint *Operand)
      : Operator(std::move(Operator)), Operand(Operand),
        Constraint(ConstraintKind::CK_UNARY) {}

private:
  string Operator;
  Constraint *Operand;
};

class PrimaryConstraint : public Constraint {
public:
  PrimaryConstraint(Feature *Feature)
      : Feature(Feature), Constraint(ConstraintKind::CK_PRIMARY) {}

private:
  Feature *Feature;
};
} // namespace vara::feature
#endif // VARA_FEATURE_CONSTRAINT_H
