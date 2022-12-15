#ifndef VARA_FEATURE_STEPFUNCTION_H
#define VARA_FEATURE_STEPFUNCTION_H

namespace vara::feature {

//===----------------------------------------------------------------------===//
//                               StepFunction
//===----------------------------------------------------------------------===//

class StepFunction {
public:
  enum class StepOperator { ADDITION, MULTIPLICATION };

  StepFunction(std::string LHS, StepOperator Op, double RHS)
      : LHS(std::move(LHS)), Op(Op), RHS(RHS) {}

  double operator()(double Value) {
    switch (Op) {
    case StepOperator::ADDITION:
      return Value + RHS;
    case StepOperator::MULTIPLICATION:
      return Value * RHS;
    }
  }

  [[nodiscard]] std::string toString() const {
    switch (Op) {
    case StepOperator::ADDITION:
      return llvm::formatv("{0} + {1}", LHS, RHS);
    case StepOperator::MULTIPLICATION:
      return llvm::formatv("{0} * {1}", LHS, RHS);
    }
  }

private:
  std::string LHS;
  StepOperator Op;
  double RHS;
};

} // namespace vara::feature

#endif // VARA_FEATURE_STEPFUNCTION_H
