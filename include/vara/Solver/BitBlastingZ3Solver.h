#ifndef VARA_SOLVER_BITBLASTINGZ3SOLVER_H_
#define VARA_SOLVER_BITBLASTINGZ3SOLVER_H_

#include "vara/Solver/Solver.h"

#include <z3++.h>

namespace vara::solver {
//===----------------------------------------------------------------------===//
//                         BitBlastingZ3Solver Class
//===----------------------------------------------------------------------===//

class FeatureEncoder {
public:
  explicit FeatureEncoder(const feature::Feature &F) : F(&F) {}
  virtual ~FeatureEncoder() = default;

  virtual std::unique_ptr<z3::expr>
  createSolverVariable(z3::context &Context) = 0;

  virtual void
  addConstraintsToSolver(z3::context &Context, z3::solver *Solver,
                         llvm::StringMap<z3::expr *> &FeatureVariables) = 0;

protected:
  const feature::Feature *F;
};

class BitBlastingZ3Solver final : public Solver {
public:
  explicit BitBlastingZ3Solver(z3::config &Cfg)
      : Context(Cfg), Solver(z3::solver(Context)) {}

  static std::unique_ptr<BitBlastingZ3Solver> create() {
    // The configuration of z3
    z3::config Cfg;
    Cfg.set("model", "true");
    Cfg.set("model-validate", "true");
    return std::make_unique<BitBlastingZ3Solver>(Cfg);
  }

  Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd,
             bool IsInAlternativeGroup = false) override;

  Result<SolverErrorCode> addFeature(const string &FeatureName) override {
    return NOT_SUPPORTED;
  }

  Result<SolverErrorCode>
  addFeature(const string &FeatureName,
             const std::vector<int64_t> &Values) override {
    return NOT_SUPPORTED;
  }

  Result<SolverErrorCode>
  removeFeature(feature::Feature &FeatureToRemove) override {
    return NOT_SUPPORTED;
  }

  Result<SolverErrorCode>
  addRelationship(const feature::Relationship &R) override {
    return NOT_IMPLEMENTED;
  }

  Result<SolverErrorCode>
  addConstraint(feature::Constraint &ConstraintToAdd) override {
    return NOT_IMPLEMENTED;
  }

  Result<SolverErrorCode>
  addMixedConstraint(feature::Constraint &ConstraintToAdd,
                     feature::FeatureModel::MixedConstraint::ExprKind ExprKind,
                     feature::FeatureModel::MixedConstraint::Req Req) override {
    return NOT_IMPLEMENTED;
  }

  Result<SolverErrorCode, bool> hasValidConfigurations() override {
    return Error(NOT_IMPLEMENTED);
  }

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getCurrentConfiguration() override {
    return Error(NOT_IMPLEMENTED);
  }

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() override {
    return Error(NOT_IMPLEMENTED);
  }

private:
  z3::context Context;
  z3::solver Solver;

  llvm::StringMap<std::unique_ptr<FeatureEncoder>> EncodedFeatures;
};

} // namespace vara::solver

#endif // VARA_SOLVER_BITBLASTINGZ3SOLVER_H_
