#include "vara/Solver/BitBlastingZ3Solver.h"

namespace vara::solver {

using namespace vara::feature;

class RootEncoder : public FeatureEncoder {
public:
  explicit RootEncoder(const Feature &F) : FeatureEncoder(F) {}

  std::unique_ptr<z3::expr>
  createSolverVariable(z3::context &Context) override {
    return std::make_unique<z3::expr>(
        Context.bool_const(F->getName().str().c_str()));
  }

  void addConstraintsToSolver(
      z3::context &Context, z3::solver *Solver,
      llvm::StringMap<z3::expr *> &FeatureVariables) override {
    Solver->add(*FeatureVariables[F->getName()]);
  }
};

class BinaryEncoder : public FeatureEncoder {
public:
  explicit BinaryEncoder(const Feature &F, const bool IsInAlternativeGroup)
      : FeatureEncoder(F), IsInAlternativeGroup(IsInAlternativeGroup) {}

  std::unique_ptr<z3::expr>
  createSolverVariable(z3::context &Context) override {
    return std::make_unique<z3::expr>(
        Context.bool_const(F->getName().str().c_str()));
  }

  void addConstraintsToSolver(
      z3::context &Context, z3::solver *Solver,
      llvm::StringMap<z3::expr *> &FeatureVariables) override {
    const auto Var = *FeatureVariables[F->getName()];
    const auto ParentVar = *FeatureVariables[F->getParentFeature()->getName()];
    Solver->add(z3::implies(Var, ParentVar));

    if (!IsInAlternativeGroup && !F->isOptional()) {
      Solver->add(z3::implies(ParentVar, Var));
    }
  }

private:
  bool IsInAlternativeGroup;
};

class ListEncoder : public FeatureEncoder {
public:
  explicit ListEncoder(const Feature &F,
                       const NumericFeature::ValueListType &ValueList)
      : FeatureEncoder(F) {}

  std::unique_ptr<z3::expr>
  createSolverVariable(z3::context &Context) override {
    assert(false); // "Not implemented!"
  }

  void addConstraintsToSolver(
      z3::context &Context, z3::solver *Solver,
      llvm::StringMap<z3::expr *> &FeatureVariables) override {
    assert(false); // "Not implemented!"
  }

private:
};

class RangeEncoder : public FeatureEncoder {
public:
  explicit RangeEncoder(const Feature &F,
                        const NumericFeature::ValueRangeType &ValueRange)
      : FeatureEncoder(F), MinVal(ValueRange.first), MaxVal(ValueRange.second),
        IsSigned(MinVal < 0) {
    BitLength =
        std::ceil(std::log2(std::max(std::abs(MinVal), std::abs(MaxVal))) + 1);
    if (IsSigned) {
      BitLength += 1;
    }
  }

  std::unique_ptr<z3::expr>
  createSolverVariable(z3::context &Context) override {
    return std::make_unique<z3::expr>(
        Context.bv_const(F->getName().str().c_str(), BitLength));
  }

  void addConstraintsToSolver(
      z3::context &Context, z3::solver *Solver,
      llvm::StringMap<z3::expr *> &FeatureVariables) override {
    const auto Var = *FeatureVariables[F->getName()];
    const auto Min = Context.bv_val(MinVal, BitLength);
    const auto Max = Context.bv_val(MaxVal, BitLength);
    Solver->add(Var >= Min);
    Solver->add(Var <= Max);
  }

  [[nodiscard]] uint16_t getBitLength() const { return BitLength; }

  void setBitLength(const uint16_t NewBitLength) { BitLength = NewBitLength; }

private:
  int64_t MinVal, MaxVal;
  bool IsSigned;
  uint16_t BitLength;
};

Result<SolverErrorCode>
BitBlastingZ3Solver::addFeature(const Feature &FeatureToAdd,
                                bool IsInAlternativeGroup) {
  const auto *Parent = FeatureToAdd.getParentFeature();
  if (Parent != nullptr &&
      EncodedFeatures.find(Parent->getName()) == EncodedFeatures.end()) {
    return PARENT_NOT_PRESENT;
  }

  if (EncodedFeatures.find(FeatureToAdd.getName()) != EncodedFeatures.end()) {
    return ALREADY_PRESENT;
  }

  const auto FeatureName = FeatureToAdd.getName().str();

  if (const auto *NumericFeature =
          llvm::dyn_cast<feature::NumericFeature>(&FeatureToAdd)) {
    const auto Values = NumericFeature->getValues();
    if (const auto *ValueList =
            std::get_if<NumericFeature::ValueListType>(&Values)) {
      EncodedFeatures.try_emplace(
          FeatureName, std::make_unique<ListEncoder>(FeatureToAdd, *ValueList));
    }
    if (const auto *ValueRange =
            std::get_if<NumericFeature::ValueRangeType>(&Values)) {
      EncodedFeatures.try_emplace(FeatureName, std::make_unique<RangeEncoder>(
                                                   FeatureToAdd, *ValueRange));
    }
  }
  if (llvm::isa<BinaryFeature>(&FeatureToAdd)) {
    EncodedFeatures.try_emplace(
        FeatureName,
        std::make_unique<BinaryEncoder>(FeatureToAdd, IsInAlternativeGroup));
    return Ok();
  }
  if (llvm::isa<RootFeature>(&FeatureToAdd)) {
    EncodedFeatures.try_emplace(FeatureName,
                                std::make_unique<RootEncoder>(FeatureToAdd));
    return Ok();
  }
  return NOT_SUPPORTED;
}

} // namespace vara::solver