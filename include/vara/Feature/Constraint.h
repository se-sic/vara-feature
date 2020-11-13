#ifndef VARA_FEATURE_CONSTRAINT_H
#define VARA_FEATURE_CONSTRAINT_H

#include "vara/Feature/Feature.h"

#include "llvm/ADT/StringRef.h"

#include <cassert>
#include <memory>
#include <sstream>
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

  void setParent(Constraint *P) { this->Parent = P; }

  [[nodiscard]] Constraint *getParent() { return Parent; }

  [[nodiscard]] Constraint *getRoot() {
    auto *R = this;
    while (R->Parent) {
      R = R->Parent;
    }
    return R;
  }

  [[nodiscard]] virtual std::string toString() const = 0;

  [[nodiscard]] virtual std::string toHTML() const { return toString(); }

  virtual void accept(ConstraintVisitor &V) = 0;

private:
  ConstraintKind Kind;
  Constraint *Parent{nullptr};
};

class BooleanConstraint {};

class NumericConstraint {};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : LeftOperand(std::move(LeftOperand)),
        RightOperand(std::move(RightOperand)),
        Constraint(ConstraintKind::CK_BINARY) {
    this->LeftOperand->setParent(this);
    this->RightOperand->setParent(this);
  }

  Constraint *getLeftOperand() { return LeftOperand.get(); }

  Constraint *getRightOperand() { return RightOperand.get(); }

  void accept(ConstraintVisitor &V) override;

protected:
  std::unique_ptr<Constraint> LeftOperand;
  std::unique_ptr<Constraint> RightOperand;
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(std::unique_ptr<Constraint> Operand)
      : Operand(std::move(Operand)), Constraint(ConstraintKind::CK_UNARY) {
    this->Operand->setParent(this);
  }

  Constraint *getOperand() { return Operand.get(); }

  void accept(ConstraintVisitor &V) override;

protected:
  std::unique_ptr<Constraint> Operand;
};

class NotConstraint : public UnaryConstraint, public BooleanConstraint {
public:
  NotConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(std::move(Operand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("!{0}", Operand->toString());
  }
};

class OrConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  OrConstraint(std::unique_ptr<Constraint> LeftOperand,
               std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} | {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class XorConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  XorConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} ^ {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class AndConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  AndConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} & {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &amp; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class EqualsConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  EqualsConstraint(std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} = {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class ImpliesConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  ImpliesConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} => {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} =&gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class EquivalenceConstraint : public BinaryConstraint,
                              public BooleanConstraint {
public:
  EquivalenceConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} <=> {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt;=&gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class NegConstraint : public UnaryConstraint, public NumericConstraint {
public:
  NegConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(std::move(Operand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("~{0}", Operand->toString());
  }
};

class AdditionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  AdditionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} + {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class SubtractionConstraint : public BinaryConstraint,
                              public NumericConstraint {
public:
  SubtractionConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} - {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class MultiplicationConstraint : public BinaryConstraint,
                                 public NumericConstraint {
public:
  MultiplicationConstraint(std::unique_ptr<Constraint> LeftOperand,
                           std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} * {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class DivisionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  DivisionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} / {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class LessConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessConstraint(std::unique_ptr<Constraint> LeftOperand,
                 std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} < {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class GreaterConstraint : public BinaryConstraint, public NumericConstraint {
public:
  GreaterConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} > {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class LessEqualConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                      std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} <= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt;= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class GreaterEqualConstraint : public BinaryConstraint,
                               public NumericConstraint {
public:
  GreaterEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                         std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(std::move(LeftOperand), std::move(RightOperand)) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} >= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &gt;= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }
};

class FeatureModelBuilder;

class PrimaryConstraint : public Constraint {
public:
  PrimaryConstraint() : Constraint(ConstraintKind::CK_PRIMARY) {}
};

class PrimaryIntegerConstraint : public PrimaryConstraint {
public:
  PrimaryIntegerConstraint(int V) : V(V) {}

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("{0}", V);
  }

private:
  int V;
};

class PrimaryFeatureConstraint : public PrimaryConstraint {
public:
  PrimaryFeatureConstraint(std::variant<Feature *, std::unique_ptr<Feature>> FV)
      : FV(std::move(FV)) {}

  [[nodiscard]] Feature *getFeature() const;

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("{0}", getFeature()->getName());
  }

  void accept(ConstraintVisitor &V) override;

private:
  friend FeatureModelBuilder;

  std::variant<Feature *, std::unique_ptr<Feature>> FV;

  void setFeature(Feature *F) { this->FV = F; }
};

class ConstraintVisitor {
public:
  virtual void visit(BinaryConstraint *C) {
    C->getLeftOperand()->accept(*this);
    C->getRightOperand()->accept(*this);
  }

  virtual void visit(UnaryConstraint *C) { C->getOperand()->accept(*this); }

  virtual void visit(PrimaryFeatureConstraint *C) {}
};

} // namespace vara::feature
#endif // VARA_FEATURE_CONSTRAINT_H
