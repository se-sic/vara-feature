#include "vara/Solver/Solver.h"

#include "z3++.h"

namespace vara::solver {

Result<SolverErrorCode>
Z3Solver::addFeature(const feature::Feature &FeatureToAdd) {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode> Z3Solver::addFeature(string FeatureName) {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode>
Z3Solver::addFeature(const string &FeatureName,
                     const std::vector<int64_t> &Values) {
  return UNSAT;
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

Result<SolverErrorCode, bool *> Z3Solver::hasValidConfigurations() {
  return NOT_IMPLEMENTED;
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getNextConfiguration() {
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

} // namespace vara::solver
