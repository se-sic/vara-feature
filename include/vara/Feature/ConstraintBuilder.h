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
  using PrecedenceTy = const unsigned int;
  inline static PrecedenceTy MinPrecedence = 0;
  inline static PrecedenceTy MaxPrecedence = 9;

  class ConstraintBuilderImpl;
  using FrameTy = std::unique_ptr<ConstraintBuilderImpl> *;

  class ConstraintBuilderImpl {
  public:
    virtual ~ConstraintBuilderImpl() = default;

    virtual std::unique_ptr<Constraint> build() = 0;

    virtual FrameTy next() { return nullptr; }
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

    FrameTy next() override { return &Operand; }

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

    FrameTy next() override { return &RightOperand; }

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
    Frames.push(std::make_tuple(ConstraintBuilder::MinPrecedence, Head));
    Head = (*Head)->next();
    return *this;
  }

  template <class ConstraintTy>
  ConstraintBuilder &binary(PrecedenceTy Precedence) {
    assert(Precedence < ConstraintBuilder::MaxPrecedence);
    assert(Precedence > ConstraintBuilder::MinPrecedence);
    if (!Head || !*Head) {
      llvm::errs() << "Syntax error: Unrecognized binary constraint.\n";
      Head = nullptr;
      return *this;
    }
    while (!Frames.empty() &&
           std::get<PrecedenceTy>(Frames.top()) <= Precedence) {
      Head = std::get<FrameTy>(Frames.top());
      Frames.pop();
    }
    assert(Head && *Head);
    *Head = std::make_unique<BinaryBuilderImpl<ConstraintTy>>(std::move(*Head));
    Frames.push(std::make_tuple(Precedence, Head));
    Head = (*Head)->next();
    return *this;
  }

public:
  /// Builds constraint.
  ///
  /// \return unique pointer for built constraint
  [[nodiscard]] std::unique_ptr<Constraint> build() {
    while (!Frames.empty() && std::get<PrecedenceTy>(Frames.top()) <
                                  ConstraintBuilder::MaxPrecedence) {
      Frames.pop();
    }
    if (!Head || !*Head || !Root || !Frames.empty()) {
      llvm::errs() << "Syntax error: Incomplete constraint.\n";
      return nullptr;
    }
    return Root->build();
  }

  /// Parenthesizes intermediate constraint.
  ///
  /// Example:
  ///     feature("A").add().feature("B")().multiply().feature("C")
  ///   produces the constraint
  ///     '((A + B) * C)'
  ConstraintBuilder &operator()() {
    while (!Frames.empty() && std::get<PrecedenceTy>(Frames.top()) <
                                  ConstraintBuilder::MaxPrecedence) {
      Frames.pop();
    }
    if (!Head || !*Head || !Frames.empty()) {
      llvm::errs() << "Syntax error: Unrecognized parentheses.\n";
      Head = nullptr;
      return *this;
    }
    assert(Root);
    Head = &Root;
    return *this;
  }

  /// Open parenthesis. Allows to add more precise parentheses.
  ///
  /// Example:
  ///     feature("A").implies().feature("B").implies()
  ///      .openPar().feature("C").implies().feature("D").closePar()
  ///   produces the constraint
  ///     '((A => B) => (C => D))'
  ///
  /// Example:
  ///     lNot().openPar().lNot().feature("A").implies().feature("B").closePar()
  ///   produces the constraint
  ///     '!(!A => B)'
  ConstraintBuilder &openPar() {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized opening parenthesis.\n";
      Head = nullptr;
      return *this;
    }
    Frames.push(std::make_tuple(ConstraintBuilder::MaxPrecedence, Head));
    return *this;
  }

  /// Close parenthesis. See \a openPar.
  ConstraintBuilder &closePar() {
    while (!Frames.empty() && std::get<PrecedenceTy>(Frames.top()) <
                                  ConstraintBuilder::MaxPrecedence) {
      Frames.pop();
    }
    if (!Head || !*Head || Frames.empty()) {
      llvm::errs() << "Syntax error: Unrecognized closing parenthesis.\n";
      Head = nullptr;
      return *this;
    }
    Head = std::get<FrameTy>(Frames.top());
    Frames.pop();
    assert(Head && *Head);
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Constant

  /// Example:
  ///     constant(42)
  ///   produces the constraint
  ///     '42'
  ConstraintBuilder &constant(int V) {
    if (!Head || *Head) {
      llvm::errs() << "Syntax error: Unrecognized constant constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head =
        std::make_unique<PrimaryBuilderImpl<PrimaryIntegerConstraint, int>>(V);
    Frames.push(std::make_tuple(ConstraintBuilder::MinPrecedence, Head));
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
    Frames.push(std::make_tuple(ConstraintBuilder::MinPrecedence, Head));
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
  ConstraintBuilder &lOr() { return binary<OrConstraint>(6); }

  /// Example:
  ///     feature("Foo").lXor().feature("Bar")
  ///   produces the constraint
  ///     '(Foo ^ Bar)'
  ConstraintBuilder &lXor() { return binary<XorConstraint>(6); }

  /// Example:
  ///     feature("Foo").lAnd().feature("Bar")
  ///   produces the constraint
  ///     '(Foo & Bar)'
  ConstraintBuilder &lAnd() { return binary<AndConstraint>(5); }

  /// Example:
  ///     feature("Foo").implies().feature("Bar")
  ///   produces the constraint
  ///     '(Foo => Bar)'
  ConstraintBuilder &implies() { return binary<ImpliesConstraint>(7); }

  /// Example:
  ///     feature("Foo").excludes().feature("Bar")
  ///   produces the constraint
  ///     '(Foo => !Bar)'
  ConstraintBuilder &excludes() { return binary<ExcludesConstraint>(7); }

  /// Example:
  ///     feature("Foo").equivalent().feature("Bar")
  ///   produces the constraint
  ///     '(Foo <=> Bar)'
  ConstraintBuilder &equivalent() { return binary<EquivalenceConstraint>(8); }

  //===--------------------------------------------------------------------===//
  // Comparison

  /// Example:
  ///     feature("Foo").equal().feature("Bar")
  ///   produces the constraint
  ///     '(Foo = Bar)'
  ConstraintBuilder &equal() { return binary<EqualConstraint>(4); }

  /// Example:
  ///     feature("Foo").notEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo != Bar)'
  ConstraintBuilder &notEqual() { return binary<NotEqualConstraint>(4); }

  /// Example:
  ///     feature("Foo").less().feature("Bar")
  ///   produces the constraint
  ///     '(Foo < Bar)'
  ConstraintBuilder &less() { return binary<LessConstraint>(3); }

  /// Example:
  ///     feature("Foo").greater().feature("Bar")
  ///   produces the constraint
  ///     '(Foo > Bar)'
  ConstraintBuilder &greater() { return binary<GreaterConstraint>(3); }

  /// Example:
  ///     feature("Foo").lessEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo <= Bar)'
  ConstraintBuilder &lessEqual() { return binary<LessEqualConstraint>(3); }

  /// Example:
  ///     feature("Foo").greaterEqual().feature("Bar")
  ///   produces the constraint
  ///     '(Foo >= Bar)'
  ConstraintBuilder &greaterEqual() {
    return binary<GreaterEqualConstraint>(3);
  }

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
  ConstraintBuilder &add() { return binary<AdditionConstraint>(2); }

  /// Example:
  ///     feature("Foo").subtract().feature("Bar")
  ///   produces the constraint
  ///     '(Foo - Bar)'
  ConstraintBuilder &subtract() { return binary<SubtractionConstraint>(2); }

  /// Example:
  ///     feature("Foo").multiply().feature("Bar")
  ///   produces the constraint
  ///     '(Foo * Bar)'
  ConstraintBuilder &multiply() { return binary<MultiplicationConstraint>(1); }

  /// Example:
  ///     feature("Foo").divide().feature("Bar")
  ///   produces the constraint
  ///     '(Foo / Bar)'
  ConstraintBuilder &divide() { return binary<DivisionConstraint>(1); }

private:
  std::unique_ptr<ConstraintBuilderImpl> Root;
  FrameTy Head;
  std::stack<std::tuple<PrecedenceTy, FrameTy>> Frames;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTBUILDER_H
