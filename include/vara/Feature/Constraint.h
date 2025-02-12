#ifndef VARA_FEATURE_CONSTRAINT_H
#define VARA_FEATURE_CONSTRAINT_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Support/FormatVariadic.h"

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
  enum class ConstraintKind {
    CK_ADDITION,
    CK_AND,
    CK_BINARY,
    CK_DIVISION,
    CK_EQUAL,
    CK_EQUIVALENCE,
    CK_EXCLUDES,
    CK_FEATURE,
    CK_GREATER,
    CK_GREATER_EQUAL,
    CK_IMPLIES,
    CK_INTEGER,
    CK_LESS,
    CK_LESS_EQUAL,
    CK_MULTIPLICATION,
    CK_NEG,
    CK_NOT,
    CK_NOT_EQUAL,
    CK_OR,
    CK_PRIMARY,
    CK_SUBTRACTION,
    CK_UNARY,
    CK_XOR
  };

  Constraint(ConstraintKind Kind) : Kind(Kind) {}
  Constraint(const Constraint &) = delete;
  Constraint &operator=(Constraint &) = delete;
  Constraint(Constraint &&) = delete;
  Constraint &operator=(Constraint &&) = delete;
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

  virtual bool accept(ConstraintVisitor &V) = 0;

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

  bool accept(ConstraintVisitor &V) override;

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

class EqualConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  EqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                  std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_EQUAL, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<EqualConstraint>(this->getLeftOperand()->clone(),
                                             this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} = {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_EQUAL;
  }
};

class NotEqualConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  NotEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                     std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_NOT_EQUAL, std::move(LeftOperand),
                         std::move(RightOperand)) {}

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<NotEqualConstraint>(
        this->getLeftOperand()->clone(), this->getRightOperand()->clone());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("({0} != {1})", LeftOperand->toString(),
                         RightOperand->toString());
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_NOT_EQUAL;
  }
};

class ImpliesConstraint : public BinaryConstraint, public BooleanConstraint {
public:
  ImpliesConstraint(std::unique_ptr<Constraint> LeftOperand,
                    std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_IMPLIES, std::move(LeftOperand),
                         std::move(RightOperand)) {}

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
      : BinaryConstraint(ConstraintKind::CK_LESS_EQUAL, std::move(LeftOperand),
                         std::move(RightOperand)) {}

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
    return C->getKind() == ConstraintKind::CK_LESS_EQUAL;
  }
};

class GreaterEqualConstraint : public BinaryConstraint,
                               public NumericConstraint {
public:
  GreaterEqualConstraint(std::unique_ptr<Constraint> LeftOperand,
                         std::unique_ptr<Constraint> RightOperand)
      : BinaryConstraint(ConstraintKind::CK_GREATER_EQUAL,
                         std::move(LeftOperand), std::move(RightOperand)) {}

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
    return C->getKind() == ConstraintKind::CK_GREATER_EQUAL;
  }
};

class UnaryConstraint : public Constraint {
public:
  UnaryConstraint(ConstraintKind Kind, std::unique_ptr<Constraint> Operand)
      : Constraint(Kind), Operand(std::move(Operand)) {
    this->Operand->setParent(this);
  }

  Constraint *getOperand() { return Operand.get(); }

  bool accept(ConstraintVisitor &V) override;

protected:
  std::unique_ptr<Constraint> Operand;
};

class NotConstraint : public UnaryConstraint, public BooleanConstraint {
public:
  NotConstraint(std::unique_ptr<Constraint> Operand)
      : UnaryConstraint(ConstraintKind::CK_NOT, std::move(Operand)) {}

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
  PrimaryIntegerConstraint(int64_t Value)
      : PrimaryConstraint(ConstraintKind::CK_INTEGER), Value(Value) {}

  [[nodiscard]] int64_t getValue() const { return Value; }

  std::unique_ptr<Constraint> clone() override {
    return std::make_unique<PrimaryIntegerConstraint>(this->getValue());
  }

  [[nodiscard]] std::string toString() const override {
    return llvm::formatv("{0}", Value);
  }

  static bool classof(const Constraint *C) {
    return C->getKind() == ConstraintKind::CK_INTEGER;
  }

  bool accept(ConstraintVisitor &V) override;

private:
  int64_t Value;
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

  bool accept(ConstraintVisitor &V) override;

private:
  std::variant<Feature *, std::unique_ptr<Feature>> FV;

  void setFeature(Feature *F) { this->FV = F; }
};

class ConstraintVisitor {
public:
  virtual ~ConstraintVisitor() = default;

  virtual bool visit(BinaryConstraint *C) {
    bool LHS = C->getLeftOperand()->accept(*this);
    bool RHS = C->getRightOperand()->accept(*this);
    return LHS && RHS;
  }

  virtual bool visit(UnaryConstraint *C) {
    return C->getOperand()->accept(*this);
  }

  virtual bool visit(PrimaryIntegerConstraint *C) { return true; }

  virtual bool visit(PrimaryFeatureConstraint *C) { return true; }
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINT_H
