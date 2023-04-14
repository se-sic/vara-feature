#ifndef VARA_FEATURE_STEPFUNCTION_H
#define VARA_FEATURE_STEPFUNCTION_H

#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormatVariadic.h"

#include <cmath>
#include <memory>
#include <variant>

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                             StepFunction Class
//===----------------------------------------------------------------------===//

class StepFunction {
public:
  enum class StepOperation { ADDITION, MULTIPLICATION, EXPONENTIATION };

  using OperandVariantType = typename std::variant<std::string, double>;

  StepFunction(OperandVariantType LHS, StepOperation Op, OperandVariantType RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  StepFunction(StepOperation Op, double RHS) : StepFunction("x", Op, RHS) {}
  StepFunction(double LHS, StepOperation Op) : StepFunction(LHS, Op, "x") {}

  template <typename T = double>
  [[nodiscard]] T next(T Value) {
    if (std::holds_alternative<double>(RHS)) {
      assert(std::holds_alternative<std::string>(LHS));
      switch (Op) {
      case StepOperation::ADDITION:
        return Value + std::get<double>(RHS);
      case StepOperation::MULTIPLICATION:
        return Value * std::get<double>(RHS);
      case StepOperation::EXPONENTIATION:
        return std::pow(Value, std::get<double>(RHS));
      }

      llvm_unreachable("Missing operation has to be implemented!");
    }
    assert(std::holds_alternative<double>(LHS));
    switch (Op) {
    case StepOperation::ADDITION:
      return std::get<double>(LHS) + Value;
    case StepOperation::MULTIPLICATION:
      return std::get<double>(LHS) * Value;
    case StepOperation::EXPONENTIATION:
      return std::pow(std::get<double>(LHS), Value);
    }

    llvm_unreachable("Missing operation has to be implemented!");
  }

  [[nodiscard]] double operator()(double Value) { return next(Value); }

  [[nodiscard]] std::string toString() const {
    if (std::holds_alternative<double>(RHS)) {
      assert(std::holds_alternative<std::string>(LHS));
      switch (Op) {
      case StepOperation::ADDITION:
        return llvm::formatv("{0} + {1}", std::get<std::string>(LHS),
                             std::get<double>(RHS));
      case StepOperation::MULTIPLICATION:
        return llvm::formatv("{0} * {1}", std::get<std::string>(LHS),
                             std::get<double>(RHS));
      case StepOperation::EXPONENTIATION:
        return llvm::formatv("{0} ^ {1}", std::get<std::string>(LHS),
                             std::get<double>(RHS));
      }
    }
    assert(std::holds_alternative<double>(LHS));
    switch (Op) {
    case StepOperation::ADDITION:
      return llvm::formatv("{0} + {1}", std::get<double>(LHS),
                           std::get<std::string>(RHS));
    case StepOperation::MULTIPLICATION:
      return llvm::formatv("{0} * {1}", std::get<double>(LHS),
                           std::get<std::string>(RHS));
    case StepOperation::EXPONENTIATION:
      return llvm::formatv("{0} ^ {1}", std::get<double>(LHS),
                           std::get<std::string>(RHS));
    }
  }

private:
  StepOperation Op;
  OperandVariantType LHS;
  OperandVariantType RHS;
};

} // namespace vara::feature

#endif // VARA_FEATURE_STEPFUNCTION_H
