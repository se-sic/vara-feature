#include "vara/Solver/Solver.h"

#include "z3++.h"

namespace vara::solver {

Result<SolverErrorCode>
Z3Solver::addFeature(const feature::Feature &FeatureToAdd) {
  // Check whether the parent feature is already added
  vara::feature::Feature *Parent = FeatureToAdd.getParentFeature();
  if (Parent != nullptr && OptionToVariableMapping.find(Parent->getName()) ==
                               OptionToVariableMapping.end()) {
    return PARENT_NOT_PRESENT;
  }

  // Add the feature
  switch (FeatureToAdd.getKind()) {
  case feature::Feature::FeatureKind::FK_NUMERIC: {
    const auto *F =
        llvm::dyn_cast<vara::feature::NumericFeature>(&FeatureToAdd);
    const auto Values = F->getValues();
    if (std::holds_alternative<vara::feature::NumericFeature::ValueListType>(
            Values)) {
      addFeature(
          F->getName(),
          std::get<vara::feature::NumericFeature::ValueListType>(Values));
    } else {
      // TODO: This has to be implemented on feature side using the constraint
      // parser
      return NOT_IMPLEMENTED;
    }
    // addFeature(FeatureToAdd.getName(), F->getValues());
    break;
  }
  case feature::Feature::FeatureKind::FK_BINARY:
    addFeature(FeatureToAdd.getName());
    // Add all constraints (i.e., implications, exclusions, parent feature)
    if (auto R = setBinaryFeatureConstraints(
            *llvm::dyn_cast<vara::feature::BinaryFeature>(&FeatureToAdd));
        !R) {
      return R;
    }
    break;
  case feature::Feature::FeatureKind::FK_ROOT:
    addFeature(FeatureToAdd.getName());
    // Add root as a constraint; root is mandatory
    Solver.add(*OptionToVariableMapping[FeatureToAdd.getName()]);
    break;
  case feature::Feature::FeatureKind::FK_UNKNOWN:
    return NOT_SUPPORTED;
  }

  return Ok();
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

Result<SolverErrorCode, std::unique_ptr<bool>>
Z3Solver::hasValidConfigurations() {
  // TODO: Try to process the constraints first
  if (!UnprocessedConstraints.empty()) {
    return NOT_ALL_CONSTRAINTS_PROCESSED;
  }
  if (Solver.check() == z3::sat) {
    return Ok(std::make_unique<bool>(true));
  }
  return Ok(std::make_unique<bool>(false));
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getNextConfiguration() {
  // Add previous configuration as a constraint
  excludeCurrentConfiguration();

  // Retrieve the next configuration
  return getCurrentConfiguration();
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

Result<SolverErrorCode>
Z3Solver::setBinaryFeatureConstraints(const feature::BinaryFeature &Feature) {
  // Add constraint to parent
  Solver.add(!*OptionToVariableMapping[Feature.getName()] ||
             *OptionToVariableMapping[Feature.getParentFeature()->getName()]);

  if (!Feature.isOptional()) {
    Solver.add(
        *OptionToVariableMapping[Feature.getName()] ||
        !*OptionToVariableMapping[Feature.getParentFeature()->getName()]);
  }

  // Process excludes
  for (const auto *C : Feature.excludes()) {
  }

  // Process implications
  for (const auto *C : Feature.implications()) {
  }
  return Ok();
}

Result<SolverErrorCode> Z3Solver::excludeCurrentConfiguration() {
  auto R = getCurrentConfiguration();
  if (!R) {
    return R.getError();
  }
  // TODO: Get the current configuration and exclude it
  return Ok();
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getCurrentConfiguration() {
  z3::model M = Solver.get_model();
  for (size_t Pos = 0; Pos < M.size(); Pos++) {
    z3::func_decl F = M[(int)Pos];
    // Make sure we have only constants here
    assert(F.arity() == 0);
  }
  return NOT_IMPLEMENTED;
}

} // namespace vara::solver
