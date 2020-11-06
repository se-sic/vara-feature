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

class BoolschesConstraint {
public:
  [[nodiscard]] virtual std::string toString() const;
};

class NumericConstraint {
public:
  [[nodiscard]] virtual std::string toString() const;
};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : LeftOperand(LeftOperand),
        RightOperand(RightOperand), Constraint(ConstraintKind::CK_BINARY) {}

  [[nodiscard]] virtual std::string toString() const;
private:
  Constraint *LeftOperand;
  Constraint *RightOperand;
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(Constraint *Operand) : Operand(Operand),
        Constraint(ConstraintKind::CK_UNARY) {}

  [[nodiscard]] virtual std::string toString() const;
private:
  Constraint *Operand;
};

class NotConstraint : public UnaryConstraint, public BoolschesConstraint {
public:
  NotConstraint(Constraint *Operand)
      : UnaryConstraint(Operand) {}

  [[nodiscard]] std::string toString() const override;
};

class OrConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  OrConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class XOrConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  XOrConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class AndConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  AndConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class EqualsConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  EqualsConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class ImpliesConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  ImpliesConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class EquivalenceConstraint : public BinaryConstraint, public BoolschesConstraint {
public:
  EquivalenceConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class NegConstraint : public UnaryConstraint, public NumericConstraint {
public:
  NegConstraint(Constraint *Operand)
      : UnaryConstraint(Operand) {}

  [[nodiscard]] std::string toString() const override;
};

class AdditionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  AdditionConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class SubtractionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  SubtractionConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class MultiplicationConstraint : public BinaryConstraint, public NumericConstraint {
public:
  MultiplicationConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class DivisionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  DivisionConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class LessConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class GreaterConstraint : public BinaryConstraint, public NumericConstraint {
public:
  GreaterConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class LessEquelsConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessEquelsConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
};

class GreaterEquelsConstraint : public BinaryConstraint, public NumericConstraint {
public:
  GreaterEquelsConstraint(Constraint *LeftOperand, Constraint *RightOperand)
      : BinaryConstraint(LeftOperand, RightOperand) {}

  [[nodiscard]] std::string toString() const override;
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
