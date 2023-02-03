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
    if (!Head) {
      llvm::errs() << "Unrecognized unary constraint.\n";
      return *this;
    }
    *Head = std::make_unique<UnaryBuilderImpl<ConstraintTy>>();
    Head = (*Head)->next();
    return *this;
  }

  template <class ConstraintTy>
  ConstraintBuilder &binary() {
    if (!Head || !*Head) {
      llvm::errs() << "Unrecognized binary constraint.\n";
      Head = nullptr;
      return *this;
    }
    *Head = std::make_unique<BinaryBuilderImpl<ConstraintTy>>(std::move(*Head));
    Head = (*Head)->next();
    return *this;
  }

public:
  [[nodiscard]] std::unique_ptr<Constraint> build() {
    if (!Head || !*Head || !Root) {
      llvm::errs() << "Incomplete constraint.\n";
      return nullptr;
    }
    return Root->build();
  }

  /// Parenthesizes constraint.
  ///
  /// Example:
  ///   The snippet
  ///     CB.constant(1).implies().feature("Foo")().implies()
  ///     CB.constant(4).implies().feature("Bar")()
  ///   produces the constraint
  ///     ((1 => Foo) => (4 => Bar))
  ConstraintBuilder &operator()() {
    if (!Head) {
      llvm::errs() << "Unrecognized constraint.\n";
      return *this;
    }
    Head = &Root;
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Constant

  /// Example:
  ///   The snippet
  ///     CB.constant(1)
  ///   produces the constraint
  ///     (1)
  ConstraintBuilder &constant(int V) {
    if (!Head) {
      llvm::errs() << "Unrecognized constant constraint.\n";
      return *this;
    }
    *Head =
        std::make_unique<PrimaryBuilderImpl<PrimaryIntegerConstraint, int>>(V);
    return *this;
  }

  //===--------------------------------------------------------------------===//
  // Feature

  /// Example:
  ///   The snippet
  ///     CB.feature("Foo")
  ///   produces the constraint
  ///     (Foo)
  ConstraintBuilder &feature(const std::string &Name) {
    if (!Head) {
      llvm::errs() << "Unrecognized feature constraint.\n";
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
  ///   The snippet
  ///     CB.lNot().feature("Foo")
  ///   produces the constraint
  ///     (!Foo)
  ConstraintBuilder &lNot() { return unary<NotConstraint>(); }

  /// Example:
  ///   The snippet
  ///     CB.feature("Foo").lOr().feature("Bar")
  ///   produces the constraint
  ///     (Foo || Bar)
  ConstraintBuilder &lOr() { return binary<OrConstraint>(); }

  ConstraintBuilder &lXor() { return binary<XorConstraint>(); }

  ConstraintBuilder &lAnd() { return binary<AndConstraint>(); }

  ConstraintBuilder &implies() { return binary<ImpliesConstraint>(); }

  ConstraintBuilder &excludes() { return binary<ExcludesConstraint>(); }

  ConstraintBuilder &equivalent() { return binary<EquivalenceConstraint>(); }

  //===--------------------------------------------------------------------===//
  // Comparison

  ConstraintBuilder &equal() { return binary<EqualConstraint>(); }

  ConstraintBuilder &notEqual() { return binary<NotEqualConstraint>(); }

  /// Example:
  ///   The snippet
  ///     CB.feature("Foo").less().feature("Bar")
  ///   produces the constraint
  ///     (Foo < Bar)
  ConstraintBuilder &less() { return binary<LessConstraint>(); }

  ConstraintBuilder &greater() { return binary<GreaterConstraint>(); }

  ConstraintBuilder &lessEqual() { return binary<LessEqualConstraint>(); }

  ConstraintBuilder &greaterEqual() { return binary<GreaterEqualConstraint>(); }

  //===--------------------------------------------------------------------===//
  // Arithmetic

  /// Example:
  ///   The snippet
  ///     CB.neg().feature("Foo")
  ///   produces the constraint
  ///     (-Foo)
  ConstraintBuilder &neg() { return unary<NegConstraint>(); }

  ConstraintBuilder &add() { return binary<AdditionConstraint>(); }

  ConstraintBuilder &subtract() { return binary<SubtractionConstraint>(); }

  ConstraintBuilder &multiply() { return binary<MultiplicationConstraint>(); }

  ConstraintBuilder &divide() { return binary<DivisionConstraint>(); }

private:
  std::unique_ptr<ConstraintBuilderImpl> Root;
  std::unique_ptr<ConstraintBuilderImpl> *Head;
};

} // namespace vara::feature

#endif // VARA_FEATURE_CONSTRAINTBUILDER_H
