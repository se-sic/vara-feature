#ifndef VARA_FEATURE_CONSTRAINT_H
#define VARA_FEATURE_CONSTRAINT_H

#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <variant>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               Constraint
//===----------------------------------------------------------------------===//

class ConstraintVisitor;

/// \brief Tree like representation of constraints between features
class Constraint {
public:
  enum class ConstraintKind { CK_BINARY, CK_UNARY, CK_PRIMARY };

  Constraint(ConstraintKind Kind) : Kind(Kind) {}
  Constraint(const Constraint &) = delete;
  Constraint &operator=(const Constraint &) = delete;
  virtual ~Constraint() = default;

  [[nodiscard]] ConstraintKind getKind() const { return Kind; };

  [[nodiscard]] virtual std::string toString() const { return ""; }

  // TODO(s9latimm): this class cannot be abstract within llvm::DenseSet
  virtual void accept(ConstraintVisitor &V) {}

private:
  ConstraintKind Kind;
};

class BooleanConstraint {};

class NumericConstraint {};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : LeftOperand(std::move(LeftOperand)),
        RightOperand(std::move(RightOperand)),
        Constraint(ConstraintKind::CK_BINARY) {}

  Constraint *getLeftOperand() { return LeftOperand.get(); }

  Constraint *getRightOperand() { return RightOperand.get(); }

  void accept(ConstraintVisitor &V) override;

private:
  std::unique_ptr<Constraint> LeftOperand;
  std::unique_ptr<Constraint> RightOperand;
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(std::unique_ptr<Constraint> Operand)
      : Operand(std::move(Operand)), Constraint(ConstraintKind::CK_UNARY) {}

  Constraint *getOperand() { return Operand.get(); }

  void accept(ConstraintVisitor &V) override;

private:
  std::unique_ptr<Constraint> Operand;
};

class NotConstraint : public UnaryConstraint, public BooleanConstraint {
public:
  NotConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(std::move(Operand)) {}
};

class OrConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  OrConstraint(std::unique_ptr<Constraint> LeftOperand,
               std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class XorConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  XorConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class AndConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  AndConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class EqualsConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  EqualsConstraint(std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class ImpliesConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  ImpliesConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class EquivalenceConstraint : public BinaryConstraint,
                              public BooleanConstraint {
public:
  EquivalenceConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class NegConstraint : public UnaryConstraint, public NumericConstraint {
public:
  NegConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(std::move(Operand)) {}
};

class AdditionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  AdditionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class SubtractionConstraint : public BinaryConstraint,
                              public NumericConstraint {
public:
  SubtractionConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class MultiplicationConstraint : public BinaryConstraint,
                                 public NumericConstraint {
public:
  MultiplicationConstraint(std::unique_ptr<Constraint> LeftOperand,
                           std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class DivisionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  DivisionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class LessConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessConstraint(std::unique_ptr<Constraint> LeftOperand,
                 std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class GreaterConstraint : public BinaryConstraint, public NumericConstraint {
public:
  GreaterConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class LessEquelsConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessEquelsConstraint(std::unique_ptr<Constraint> LeftOperand,
                       std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class GreaterEquelsConstraint : public BinaryConstraint,
                                public NumericConstraint {
public:
  GreaterEquelsConstraint(std::unique_ptr<Constraint> LeftOperand,
                          std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}
};

class Feature;
class FeatureModelBuilder;

class PrimaryConstraint : public Constraint {
public:
  PrimaryConstraint(std::variant<Feature *, std::string> FV)
      : FV(std::move(FV)), Constraint(ConstraintKind::CK_PRIMARY) {}

  std::variant<Feature *, std::string> getFeature() { return FV; }

  void accept(ConstraintVisitor &V) override;

private:
  friend FeatureModelBuilder;

  std::variant<Feature *, std::string> FV;

  void setFeature(Feature *F) { this->FV = F; }
};

class ConstraintVisitor {
public:
  virtual void visit(BinaryConstraint *C) {
    C->getLeftOperand()->accept(*this);
    C->getRightOperand()->accept(*this);
  }

  virtual void visit(UnaryConstraint *C) { C->getOperand()->accept(*this); }

  virtual void visit(PrimaryConstraint *C) {}
};

} // namespace vara::feature
#endif // VARA_FEATURE_CONSTRAINT_H
