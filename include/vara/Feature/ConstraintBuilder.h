#ifndef VARA_FEATURE_CONSTRAINTBUILDER_H
#define VARA_FEATURE_CONSTRAINTBUILDER_H

#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                              ConstraintBuilder
//===----------------------------------------------------------------------===//

class ConstraintBuilder {
public:
  ConstraintBuilder() : Root(nullptr), Head(&Root) {}
  ConstraintBuilder(const ConstraintBuilder &) = delete;
  ConstraintBuilder &operator=(const ConstraintBuilder &) = delete;
  ConstraintBuilder(ConstraintBuilder &&) = delete;
  ConstraintBuilder &operator=(ConstraintBuilder &&) = delete;
  ~ConstraintBuilder() = default;

private:
  class ConstraintBuilderImpl {
  public:
    virtual ~ConstraintBuilderImpl() = default;

    virtual std::unique_ptr<Constraint> build() = 0;

    virtual std::unique_ptr<ConstraintBuilderImpl> *next() { return nullptr; }
  };

  template <class ConstraintTy, class ValueTy>
  class PrimaryBuilderImpl : public ConstraintBuilderImpl {

  public:
    PrimaryBuilderImpl(ValueTy Value) : Value(std::move(Value)) {}

    std::unique_ptr<Constraint> build() override {
      return std::make_unique<ConstraintTy>(std::move(Value));
    }

  private:
    ValueTy Value;
  };

  template <class ConstraintTy>
  class UnaryBuilderImpl : public ConstraintBuilderImpl {

  public:
    UnaryBuilderImpl() : Operand(nullptr) {}

    std::unique_ptr<ConstraintBuilderImpl> *next() override { return &Operand; }

    std::unique_ptr<Constraint> build() override {
      return std::make_unique<ConstraintTy>(std::move(Operand->build()));
    }

  private:
    std::unique_ptr<ConstraintBuilderImpl> Operand;
  };

  template <class ConstraintTy>
  class BinaryBuilderImpl : public ConstraintBuilderImpl {

  public:
    BinaryBuilderImpl(std::unique_ptr<ConstraintBuilderImpl> LeftOperand)
        : LeftOperand(std::move(LeftOperand)), RightOperand(nullptr) {}

    std::unique_ptr<ConstraintBuilderImpl> *next() override {
      return &RightOperand;
    }

    std::unique_ptr<Constraint> build() override {
      return std::make_unique<ConstraintTy>(std::move(LeftOperand->build()),
                                            std::move(RightOperand->build()));
    }

  private:
    std::unique_ptr<ConstraintBuilderImpl> LeftOperand;
    std::unique_ptr<ConstraintBuilderImpl> RightOperand;
  };

  template <class ConstraintTy>
  ConstraintBuilder &unary() {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized unary constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head = std::make_unique<UnaryBuilderImpl<ConstraintTy>>();
    Head = (*Head)->next();
    return *this;
  }

  template <class ConstraintTy>
  ConstraintBuilder &binary() {
    if (!Head || !*Head) {
      llvm::errs() << "Syntax error: Unrecognized binary constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head = std::make_unique<BinaryBuilderImpl<ConstraintTy>>(std::move(*Head));
    Head = (*Head)->next();
    return *this;
  }

public:
  /// Builds constraint.
  ///
  /// \return unique pointer for built constraint
  [[nodiscard]] std::unique_ptr<Constraint> build() {
    if (!Head || !*Head || !Root || !Parentheses.empty()) {
      llvm::errs() << "Syntax error: Incomplete constraint.\n";
      return nullptr;
    }
    return Root->build();
  }

  /// Parenthesizes intermediate constraint.
  ///
  /// Example:
  ///     feature("A").implies().feature("B")()
  ///      .implies().feature("C").implies().feature("D")
  ///   produces the constraint
  ///     '((A => B) => (C => D))'
  ConstraintBuilder &operator()() {
    if (!Head || !*Head || !Parentheses.empty()) {
      llvm::errs() << "Syntax error: Unrecognized parentheses.\n";
      Head = nullptr;
      return *this;
    }
    Head = &Root;
    return *this;
  }

  /// Open parenthesis. Allows to add more precise parentheses.
  ///
  /// Example:
  ///     left().feature("A").implies().feature("B").right()
  ///      .implies().feature("C").implies().feature("D")
  ///   produces the constraint
  ///     '((A => B) => (C => D))'
  ///
  /// CAVE: These may be required for unary operators.
  ///
  /// Example:
  ///     lNot().left().lNot().feature("A").right().implies().feature("B")
  ///   produces the constraint
  ///     '!(!A => B)'
  ConstraintBuilder &left() {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized left parenthesis.\n";
      Head = nullptr;
      return *this;
    }
    Parentheses.push(Head);
    return *this;
  }

  /// Close parenthesis. See \a left.
  ///
  /// CAVE: If used without a succeeding operator or in conjunction with unary
  ///  expressions, this may not yield the desired result.
  ///
  /// Example:
  ///     lNot().left().lNot().feature("A").implies().feature("B").right()
  ///   produces the constraint
  ///     '!!(A => B)'
  ConstraintBuilder &right() {
    if (!Head || !*Head || Parentheses.empty()) {
      llvm::errs() << "Syntax error: Unrecognized right parenthesis.\n";
      Head = nullptr;
      return *this;
    }
    Head = Parentheses.top();
    Parentheses.pop();
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Constant

  /// Example:
  ///     constant(42)
  ///   produces the constraint
  ///     42
  ConstraintBuilder &constant(int V) {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized constant constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head =
        std::make_unique<PrimaryBuilderImpl<PrimaryIntegerConstraint, int>>(V);
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Feature

  /// Example:
  ///     feature("Foo")
  ///   produces the constraint
  ///     'Foo'
  ConstraintBuilder &feature(const std::string &Name) {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized feature constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head = std::make_unique<
        PrimaryBuilderImpl<PrimaryFeatureConstraint, std::unique_ptr<Feature>>>(
        std::make_unique<Feature>(Name));
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Logical

  /// Example:
  ///     lNot().feature("Foo")
  ///   produces the constraint
  ///     '!Foo'
  ConstraintBuilder &lNot() { return unary<NotConstraint>(); }

  /// Example:
  ///     feature("Foo").lOr().feature("Bar")
  ///   produces the constraint
  ///     '(Foo | Bar)'
  ConstraintBuilder &lOr() { return binary<OrConstraint>(); }

  /// Example:
  ///     feature("Foo").lXor().feature("Bar")
  ///   produces the constraint
  ///     '(Foo ^ Bar)'
  ConstraintBuilder &lXor() { return binary<XorConstraint>(); }

  /// Example:
  ///     feature("Foo").lAnd().feature("Bar")
  ///   produces the constraint
  ///     '(Foo & Bar)'
  ConstraintBuilder &lAnd() { return binary<AndConstraint>(); }

  /// Example:
  ///     feature("Foo").implies().feature("Bar")
  ///   produces the constraint
  ///     '(Foo => Bar)'
  ConstraintBuilder &implies() { return binary<ImpliesConstraint>(); }

  /// Example:
  ///     feature("Foo").excludes().feature("Bar")
  ///   produces the constraint
  ///     '(Foo => !Bar)'
  ConstraintBuilder &excludes() { return binary<ExcludesConstraint>(); }

  /// Example:
  ///     feature("Foo").equivalent().feature("Bar")
  ///   produces the constraint
  ///     '(Foo <=> Bar)'
  ConstraintBuilder &equivalent() { return binary<EquivalenceConstraint>(); }

  //===--------------------------------------------------------------------===//
  // Comparison

  /// Example:
  ///     feature("Foo").equal().feature("Bar")
  ///   produces the constraint
  ///     '(Foo = Bar)'
  ConstraintBuilder &equal() { return binary<EqualConstraint>(); }

  /// Example:
  ///     feature("Foo").notEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo != Bar)'
  ConstraintBuilder &notEqual() { return binary<NotEqualConstraint>(); }

  /// Example:
  ///     feature("Foo").less().feature("Bar")
  ///   produces the constraint
  ///     '(Foo < Bar)'
  ConstraintBuilder &less() { return binary<LessConstraint>(); }

  /// Example:
  ///     feature("Foo").greater().feature("Bar")
  ///   produces the constraint
  ///     '(Foo > Bar)'
  ConstraintBuilder &greater() { return binary<GreaterConstraint>(); }

  /// Example:
  ///     feature("Foo").lessEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo <= Bar)'
  ConstraintBuilder &lessEqual() { return binary<LessEqualConstraint>(); }

  /// Example:
  ///     feature("Foo").greaterEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo >= Bar)'
  ConstraintBuilder &greaterEqual() { return binary<GreaterEqualConstraint>(); }

  //===--------------------------------------------------------------------===//
  // Arithmetic

  /// Example:
  ///     neg().feature("Foo")
  ///   produces the constraint
  ///     '~Foo'
  ConstraintBuilder &neg() { return unary<NegConstraint>(); }

  /// Example:
  ///     feature("Foo").add().feature("Bar")
  ///   produces the constraint
  ///     '(Foo + Bar)'
  ConstraintBuilder &add() { return binary<AdditionConstraint>(); }

  /// Example:
  ///     feature("Foo").subtract().feature("Bar")
  ///   produces the constraint
  ///     '(Foo - Bar)'
  ConstraintBuilder &subtract() { return binary<SubtractionConstraint>(); }

  /// Example:
  ///     feature("Foo").multiply().feature("Bar")
  ///   produces the constraint
  ///     '(Foo * Bar)'
  ConstraintBuilder &multiply() { return binary<MultiplicationConstraint>(); }

  /// Example:
  ///     feature("Foo").divide().feature("Bar")
  ///   produces the constraint
  ///     '(Foo / Bar)'
  ConstraintBuilder &divide() { return binary<DivisionConstraint>(); }

private:
  std::unique_ptr<ConstraintBuilderImpl> Root;
  std::unique_ptr<ConstraintBuilderImpl> *Head;
  std::stack<std::unique_ptr<ConstraintBuilderImpl> *> Parentheses;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTBUILDER_H
