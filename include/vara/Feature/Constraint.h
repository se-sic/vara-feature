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
  ConstraintKind Kind;
};

class ConstraintOperator {
public:
  /// TODO: Numeric Operators are missing (e.g. GREATER, LESS, MINUS, ADD, MULTIPLY, DIVISION)
  enum class OperatorKind { OK_NOT, OK_NEGATION, OK_AND, OK_OR, OK_XOR, OK_EQUALS, OK_IMPLIES, OK_EQUIVALENCE };
  ConstraintOperator(OperatorKind Operator) : Operator(Operator) {};

  [[nodiscard]] std::string toString() {
    switch(Operator) {
    case OperatorKind::OK_NOT: return "!";
    case OperatorKind::OK_NEGATION: return "~";
    case OperatorKind::OK_AND: return "&";
    case OperatorKind::OK_OR: return "v";
    case OperatorKind::OK_EQUALS: return "=";
    case OperatorKind::OK_IMPLIES: return "=>";
    case OperatorKind::OK_EQUIVALENCE: return "<=>";
    case OperatorKind::OK_XOR: return "^";
    default: return "";
    }
  };

private:
  OperatorKind Operator;
};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(ConstraintOperator Operator, Constraint *LeftOperand,
                   Constraint *RightOperand)
      : Operator(Operator), LeftOperand(LeftOperand),
        RightOperand(RightOperand), Constraint(ConstraintKind::CK_BINARY) {}

private:
/// TODO: maybe we use a variant type to allow for operators that are not added as enumkind
  ConstraintOperator Operator;
  Constraint *LeftOperand;
  Constraint *RightOperand;
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(ConstraintOperator Operator, Constraint *Operand)
      : Operator(Operator), Operand(Operand),
        Constraint(ConstraintKind::CK_UNARY) {}

private:
  ConstraintOperator Operator;
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
