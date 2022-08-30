#include "vara/Solver/Solver.h"

#include "z3++.h"

namespace vara::solver {

Result<SolverErrorCode>
Z3Solver::addFeature(const feature::Feature &FeatureToAdd) {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode> Z3Solver::addFeature(const string &FeatureName) {
  if (OptionToVariableMapping.find(FeatureName) !=
      OptionToVariableMapping.end()) {
    return ALREADY_PRESENT;
  }
  z3::expr Feature = Context.bool_const(FeatureName.c_str());
  OptionToVariableMapping.insert(
      std::make_pair(FeatureName, std::make_unique<z3::expr>(Feature)));
  return Ok();
}

Result<SolverErrorCode>
Z3Solver::addFeature(const string &FeatureName,
                     const std::vector<int64_t> &Values) {
  if (OptionToVariableMapping.find(llvm::StringRef(FeatureName)) !=
      OptionToVariableMapping.end()) {
    return ALREADY_PRESENT;
  }
  z3::expr Feature = Context.int_const(FeatureName.c_str());
  OptionToVariableMapping.insert(
      std::make_pair(FeatureName, std::make_unique<z3::expr>(Feature)));

  // Add the numeric values as constraints
  z3::expr Constraint = Context.bool_val(false);
  for (int64_t Value : Values) {
    Constraint = Constraint || (Feature == Context.int_val(Value));
  }
  Solver.add(Constraint);

  return Ok();
}

Result<SolverErrorCode>
Z3Solver::removeFeature(const feature::Feature &FeatureToRemove) {
  return NOT_SUPPORTED;
}

Result<SolverErrorCode>
Z3Solver::addConstraint(const feature::Constraint &ConstraintToAdd) {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode>
Z3Solver::removeConstraint(const feature::Constraint &ConstraintToRemove) {
  return NOT_SUPPORTED;
}

Result<SolverErrorCode> Z3Solver::hasValidConfigurations() {
  // TODO: Try to process the constraints first
  if (!UnprocessedConstraints.empty()) {
    return NOT_ALL_CONSTRAINTS_PROCESSED;
  }
  if (Solver.check() == z3::sat) {
    return Ok();
  }
  return UNSAT;
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getNextConfiguration() {
  // Add previous configuration as a constraint

  // Retrieve the next configuration
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode> Z3Solver::resetConfigurationIterator() {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode, u_int64_t *> Z3Solver::getNumberValidConfigurations() {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode,
       std::vector<std::unique_ptr<vara::feature::Configuration>> *>
Z3Solver::getAllValidConfigurations() {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode> Z3Solver::excludeCurrentConfiguration() {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getCurrentConfiguration() {
  return NOT_IMPLEMENTED;
}

} // namespace vara::solver
