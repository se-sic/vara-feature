#ifndef VARA_FEATURE_CONSTRAINT_H
#define VARA_FEATURE_CONSTRAINT_H

#include "vara/Utils/VariantUtil.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FormatVariadic.h"

#include <cassert>
#include <llvm/Support/Casting.h>
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
  enum class ConstraintKind {
    CK_BINARY,
    CK_OR,
    CK_XOR,
    CK_AND,
    CK_EQUALS,
    CK_IMPLIES,
    CK_EXCLUDES,
    CK_EQUIVALENCE,
    CK_ADDITION,
    CK_SUBTRACTION,
    CK_MULTIPLICATION,
    CK_DIVISION,
    CK_LESS,
    CK_GREATER,
    CK_LESSEQUAL,
    CK_GREATEREQUAL,
    CK_UNARY,
    CK_NOT,
    CK_NEG,
    CK_PRIMARY,
    CK_INTEGER,
    CK_FEATURE
  };

  Constraint(ConstraintKind Kind) : Kind(Kind) {}
  Constraint(const Constraint &) = delete;
  Constraint &operator=(Constraint &) = delete;
  Constraint(Constraint &&) = delete;
  Constraint &operator=(Constraint &&) = delete;
  bool operator==(const Constraint &Other) const {
    return this->equals(Other);
  };
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

  /// Recursively clones constraint tree. If this tree contains special nodes
  /// (like \a PrimaryFeatureConstraint) those may need to be updated
  /// afterwards.
  ///
  /// \return root of cloned constraint tree
  [[nodiscard]] virtual std::unique_ptr<Constraint> clone() = 0;

  [[nodiscard]] virtual std::string toString() const = 0;

  [[nodiscard]] virtual std::string toHTML() const { return toString(); }

  virtual void accept(ConstraintVisitor &V) = 0;

  [[nodiscard]] virtual bool equals(const Constraint &Other) const = 0;

private:
  const ConstraintKind Kind;
  Constraint *Parent{nullptr};
};

class BooleanConstraint {};

class NumericConstraint {};

class BinaryConstraint : public Constraint {
public:
  BinaryConstraint(ConstraintKind Kind, std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : Constraint(Kind), LeftOperand(std::move(LeftOperand)),
        RightOperand(std::move(RightOperand)) {
    this->LeftOperand->setParent(this);
    this->RightOperand->setParent(this);
  }

  [[nodiscard]] Constraint *getLeftOperand() const { return LeftOperand.get(); }

  [[nodiscard]] Constraint *getRightOperand() const {
    return RightOperand.get();
  }

  void accept(ConstraintVisitor &V) override;

protected:
  std::unique_ptr<Constraint> LeftOperand;
  std::unique_ptr<Constraint> RightOperand;
};

class OrConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  OrConstraint(std::unique_ptr<Constraint> LeftOperand,
               std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_OR, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherOr = llvm::dyn_cast<OrConstraint>(&Other);
    if (OtherOr) {
      return LeftOperand == OtherOr->LeftOperand &&
             RightOperand == OtherOr->RightOperand;
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<OrConstraint>(this->getLeftOperand()->clone(),
                                          this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} | {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_OR;
  }
};

class XorConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  XorConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_XOR, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherXor = llvm::dyn_cast<XorConstraint>(&Other);
    if (OtherXor) {
      return LeftOperand == OtherXor->LeftOperand &&
             RightOperand == OtherXor->RightOperand;
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<XorConstraint>(this->getLeftOperand()->clone(),
                                           this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} ^ {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_XOR;
  }
};

class AndConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  AndConstraint(std::unique_ptr<Constraint> LeftOperand,
                std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_AND, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherAnd = llvm::dyn_cast<AndConstraint>(&Other);
    if (OtherAnd) {
      return LeftOperand == OtherAnd->LeftOperand &&
             RightOperand == OtherAnd->RightOperand;
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<AndConstraint>(this->getLeftOperand()->clone(),
                                           this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} & {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &amp; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_AND;
  }
};

class EqualsConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  EqualsConstraint(std::unique_ptr<Constraint> LeftOperand,
                   std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_EQUALS, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherEquals = llvm::dyn_cast<EqualsConstraint>(&Other);
    if (OtherEquals) {
      return LeftOperand == OtherEquals->LeftOperand &&
             RightOperand == OtherEquals->RightOperand;
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<EqualsConstraint>(this->getLeftOperand()->clone(),
                                              this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} = {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_EQUALS;
  }
};

class ImpliesConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  ImpliesConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_IMPLIES, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherImplies = llvm::dyn_cast<ImpliesConstraint>(&Other);
    if (OtherImplies) {
      return LeftOperand->equals(*(OtherImplies->LeftOperand)) &&
             RightOperand->equals(*(OtherImplies->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<ImpliesConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} => {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} =&gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_IMPLIES;
  }
};

class ExcludesConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  ExcludesConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_EXCLUDES, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherExcludes = llvm::dyn_cast<ExcludesConstraint>(&Other);
    if (OtherExcludes) {
      return LeftOperand->equals(*(OtherExcludes->LeftOperand)) &&
             RightOperand->equals(*(OtherExcludes->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<ExcludesConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} => !{1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} =&gt; !{1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_EXCLUDES;
  }
};

class EquivalenceConstraint : public BinaryConstraint,
                              public BooleanConstraint {
public:
  EquivalenceConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_EQUIVALENCE, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherEquivalence = llvm::dyn_cast<EquivalenceConstraint>(&Other);
    if (OtherEquivalence) {
      return LeftOperand->equals(*(OtherEquivalence->LeftOperand)) &&
             RightOperand->equals(*(OtherEquivalence->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<EquivalenceConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} <=> {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt;=&gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_EQUIVALENCE;
  }
};

class AdditionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  AdditionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_ADDITION, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherAddition = llvm::dyn_cast<AdditionConstraint>(&Other);
    if (OtherAddition) {
      return LeftOperand->equals(*(OtherAddition->LeftOperand)) &&
             RightOperand->equals(*(OtherAddition->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<AdditionConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} + {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_ADDITION;
  }
};

class SubtractionConstraint : public BinaryConstraint,
                              public NumericConstraint {
public:
  SubtractionConstraint(std::unique_ptr<Constraint> LeftOperand,
                        std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_SUBTRACTION, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherSubtraction = llvm::dyn_cast<SubtractionConstraint>(&Other);
    if (OtherSubtraction) {
      return LeftOperand->equals(*(OtherSubtraction->LeftOperand)) &&
             RightOperand->equals(*(OtherSubtraction->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<SubtractionConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} - {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_SUBTRACTION;
  }
};

class MultiplicationConstraint : public BinaryConstraint,
                                 public NumericConstraint {
public:
  MultiplicationConstraint(std::unique_ptr<Constraint> LeftOperand,
                           std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_MULTIPLICATION,
                         std::move(LeftOperand), std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherMultiplication =
        llvm::dyn_cast<MultiplicationConstraint>(&Other);
    if (OtherMultiplication) {
      return LeftOperand->equals(*(OtherMultiplication->LeftOperand)) &&
             RightOperand->equals(*(OtherMultiplication->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<MultiplicationConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} * {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_MULTIPLICATION;
  }
};

class DivisionConstraint : public BinaryConstraint, public NumericConstraint {
public:
  DivisionConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_DIVISION, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherDivision = llvm::dyn_cast<DivisionConstraint>(&Other);
    if (OtherDivision) {
      return LeftOperand->equals(*(OtherDivision->LeftOperand)) &&
             RightOperand->equals(*(OtherDivision->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<DivisionConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} / {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_DIVISION;
  }
};

class LessConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessConstraint(std::unique_ptr<Constraint> LeftOperand,
                 std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_LESS, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherLess = llvm::dyn_cast<LessConstraint>(&Other);
    if (OtherLess) {
      return LeftOperand->equals(*(OtherLess->LeftOperand)) &&
             RightOperand->equals(*(OtherLess->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<LessConstraint>(this->getLeftOperand()->clone(),
                                            this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} < {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_LESS;
  }
};

class GreaterConstraint : public BinaryConstraint, public NumericConstraint {
public:
  GreaterConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_GREATER, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherGreater = llvm::dyn_cast<GreaterConstraint>(&Other);
    if (OtherGreater) {
      return LeftOperand->equals(*(OtherGreater->LeftOperand)) &&
             RightOperand->equals(*(OtherGreater->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<GreaterConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} > {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &gt; {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_GREATER;
  }
};

class LessEqualConstraint : public BinaryConstraint, public NumericConstraint {
public:
  LessEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                      std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_LESSEQUAL, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherLessEqual = llvm::dyn_cast<LessEqualConstraint>(&Other);
    if (OtherLessEqual) {
      return LeftOperand->equals(*(OtherLessEqual->LeftOperand)) &&
             RightOperand->equals(*(OtherLessEqual->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<LessEqualConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} <= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &lt;= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_LESSEQUAL;
  }
};

class GreaterEqualConstraint : public BinaryConstraint,
                               public NumericConstraint {
public:
  GreaterEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                         std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_GREATEREQUAL,
                         std::move(LeftOperand), std::move(RightOperand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherGreaterEqual = llvm::dyn_cast<GreaterEqualConstraint>(&Other);
    if (OtherGreaterEqual) {
      return LeftOperand->equals(*(OtherGreaterEqual->LeftOperand)) &&
             RightOperand->equals(*(OtherGreaterEqual->RightOperand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<GreaterEqualConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} >= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  [[nodiscard]] std::string toHTML() const override {
    return llvm::formatv("({0} &gt;= {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_GREATEREQUAL;
  }
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(ConstraintKind Kind, std::unique_ptr<Constraint> Operand)
      : Constraint(Kind), Operand(std::move(Operand)) {
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
      : UnaryConstraint(ConstraintKind::CK_NOT, std::move(Operand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherNot = llvm::dyn_cast<NotConstraint>(&Other);
    if (OtherNot) {
      return Operand->equals(*(OtherNot->Operand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<NotConstraint>(this->getOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("!{0}", Operand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_NOT;
  }
};

class NegConstraint : public UnaryConstraint, public NumericConstraint {
public:
  NegConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(ConstraintKind::CK_NEG, std::move(Operand)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherNeg = llvm::dyn_cast<NegConstraint>(&Other);
    if (OtherNeg) {
      return Operand->equals(*(OtherNeg->Operand));
    }
    return false;
  }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<NegConstraint>(this->getOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("~{0}", Operand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_NEG;
  }
};

class FeatureModelBuilder;

class PrimaryConstraint : public Constraint {
public:
  PrimaryConstraint(ConstraintKind Kind) : Constraint(Kind) {}
};

class PrimaryIntegerConstraint : public PrimaryConstraint {
public:
  PrimaryIntegerConstraint(int Value)
      : PrimaryConstraint(ConstraintKind::CK_INTEGER), Value(Value) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherPrimaryInteger =
        llvm::dyn_cast<PrimaryIntegerConstraint>(&Other);
    if (OtherPrimaryInteger) {
      return this->Value == OtherPrimaryInteger->Value;
    }
    return false;
  }

  [[nodiscard]] int getValue() const { return Value; }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<PrimaryIntegerConstraint>(this->getValue());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("{0}", Value);
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_INTEGER;
  }

  void accept(ConstraintVisitor &V) override;

private:
  int Value;
};

class Feature;

namespace detail {
class FeatureModelModification;
} // namespace detail

class PrimaryFeatureConstraint : public PrimaryConstraint {
  friend detail::FeatureModelModification;

public:
  PrimaryFeatureConstraint(std::variant<Feature *, std::unique_ptr<Feature>> FV)
      : PrimaryConstraint(ConstraintKind::CK_FEATURE), FV(std::move(FV)) {}

  bool equals(const Constraint &Other) const override {
    auto *OtherPrimaryFeature =
        llvm::dyn_cast<PrimaryFeatureConstraint>(&Other);
    if (OtherPrimaryFeature) {
      // Since FV can contains pointers and equals comparison does not compare
      // values for pointers, this needs to be done manually
      Feature *ActualFeature;
      std::visit(
          Overloaded{[&ActualFeature](Feature *F) { ActualFeature = F; },
                     [&ActualFeature](const std::unique_ptr<Feature> &F) {
                       ActualFeature = F.get();
                     }},
          FV);
      Feature *ActualOtherFeature;
      std::visit(
          Overloaded{
              [&ActualOtherFeature](Feature *F) { ActualOtherFeature = F; },
              [&ActualOtherFeature](const std::unique_ptr<Feature> &F) {
                ActualOtherFeature = F.get();
              }},
          OtherPrimaryFeature->FV);

      // TODO: f-701 it seems that here are really two different values inside.
      // Maybe copy in addImpliedExcludeConstraint fails
      return (&(*ActualFeature)) == (&(*ActualOtherFeature));
    }
    return false;
  }

  [[nodiscard]] Feature *getFeature() const;

  /// Creates copy of feature constraint with a shallow feature. If cloning
  /// an enclosing data structure with additional feature information, the
  /// feature pointer may has to be updated afterwards.
  ///
  /// \return copy of constraint
  std::unique_ptr<Constraint> clone() override;

  [[nodiscard]] std::string toString() const override;

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_FEATURE;
  }

  void accept(ConstraintVisitor &V) override;

private:
  std::variant<Feature *, std::unique_ptr<Feature>> FV;

  void setFeature(Feature *F) { this->FV = F; }
};

class ConstraintVisitor {
public:
  virtual ~ConstraintVisitor() = default;

  virtual void visit(BinaryConstraint *C) {
    C->getLeftOperand()->accept(*this);
    C->getRightOperand()->accept(*this);
  }

  virtual void visit(UnaryConstraint *C) { C->getOperand()->accept(*this); }

  virtual void visit(PrimaryIntegerConstraint *C) {}

  virtual void visit(PrimaryFeatureConstraint *C) {}
};

} // namespace vara::feature
#endif // VARA_FEATURE_CONSTRAINT_H
