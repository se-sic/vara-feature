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

  if (OptionToVariableMapping.find(FeatureToAdd.getName()) !=
      OptionToVariableMapping.end()) {
    return ALREADY_PRESENT;
  }

  // Add the feature
  switch (FeatureToAdd.getKind()) {
  case feature::Feature::FeatureKind::FK_NUMERIC: {
    const auto *F =
        llvm::dyn_cast<vara::feature::NumericFeature>(&FeatureToAdd);
    if (!F) {
      return NOT_SUPPORTED;
    }
    const auto Values = F->getValues();
    if (std::holds_alternative<vara::feature::NumericFeature::ValueListType>(
            Values)) {
      if (auto R = addFeature(
              F->getName().str(),
              std::get<vara::feature::NumericFeature::ValueListType>(Values));
          !R) {
        return R;
      }
    } else {
      // TODO: This has to be implemented on feature side using the constraint
      // parser
      return NOT_IMPLEMENTED;
    }
    break;
  }
  case feature::Feature::FeatureKind::FK_BINARY: {
    addFeature(FeatureToAdd.getName().str());
    // Add all constraints (i.e., implications, exclusions, parent feature)
    if (!llvm::dyn_cast<vara::feature::BinaryFeature>(&FeatureToAdd)) {
      return NOT_SUPPORTED;
    }
    auto R = setBinaryFeatureConstraints(
        *llvm::dyn_cast<vara::feature::BinaryFeature>(&FeatureToAdd));
    if (!R) {
      return R;
    }
    break;
  }
  case feature::Feature::FeatureKind::FK_ROOT:
    addFeature(FeatureToAdd.getName().str());
    // Add root as a constraint; root is mandatory
    Solver->add(*OptionToVariableMapping[FeatureToAdd.getName()]);
    break;
  case feature::Feature::FeatureKind::FK_UNKNOWN:
    return NOT_SUPPORTED;
  }

  Solver->push();
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
  Solver->add(Constraint);

  return Ok();
}

Result<SolverErrorCode>
Z3Solver::removeFeature(feature::Feature &FeatureToRemove) {
  return NOT_SUPPORTED;
}

Result<SolverErrorCode>
Z3Solver::addRelationship(const feature::Relationship &R) {
  const auto *Parent = (const feature::Feature *)R.getParent();
  auto ParentOption = Context.bool_const(Parent->getName().str().c_str());
  z3::expr_vector V(Context);
  for (const auto &Child : R.children()) {
    const auto *ChildFeature = (const feature::Feature *)Child;
    V.push_back(Context.bool_const(ChildFeature->getName().str().c_str()));
  }
  switch (R.getKind()) {
  case feature::Relationship::RelationshipKind::RK_ALTERNATIVE:
    Solver->add(z3::implies(ParentOption, z3::atmost(V, 1) & z3::mk_or(V)));
    break;
  case feature::Relationship::RelationshipKind::RK_OR:
    Solver->add(z3::implies(ParentOption, z3::mk_or(V)));
    break;
  }
  return Ok();
}

Result<SolverErrorCode>
Z3Solver::addConstraint(feature::Constraint &ConstraintToAdd) {
  SolverConstraintVisitor SCV(this);
  bool Succ = SCV.addConstraint(&ConstraintToAdd);
  if (!Succ) {
    return SolverErrorCode::NOT_SUPPORTED;
  }
  return Ok();
}

Result<SolverErrorCode>
Z3Solver::removeConstraint(feature::Constraint &ConstraintToRemove) {
  return NOT_SUPPORTED;
}

Result<SolverErrorCode, std::unique_ptr<bool>>
Z3Solver::hasValidConfigurations() {
  // TODO: Try to process the constraints first
  if (!UnprocessedConstraints.empty()) {
    return NOT_ALL_CONSTRAINTS_PROCESSED;
  }
  if (Solver->check() == z3::sat) {
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

Result<SolverErrorCode, std::unique_ptr<uint64_t>>
Z3Solver::getNumberValidConfigurations() {
  Solver->push();
  std::unique_ptr<uint64_t> Count = std::make_unique<uint64_t>(0);
  while (Solver->check() == z3::sat) {
    excludeCurrentConfiguration();
    (*Count)++;
  }
  Solver->pop();
  return Count;
}

Result<
    SolverErrorCode,
    std::unique_ptr<std::vector<std::unique_ptr<vara::feature::Configuration>>>>
Z3Solver::getAllValidConfigurations() {
  Solver->push();
  std::unique_ptr<std::vector<std::unique_ptr<vara::feature::Configuration>>>
      Vector(new std::vector<std::unique_ptr<vara::feature::Configuration>>);
  while (Solver->check() == z3::sat) {
    auto Config = getCurrentConfiguration().extractValue();
    Vector->insert(Vector->begin(), std::move(Config));
    excludeCurrentConfiguration();
  }
  Solver->pop();
  return Vector;
}

Result<SolverErrorCode>
Z3Solver::setBinaryFeatureConstraints(const feature::BinaryFeature &Feature) {
  // Add constraint to parent
  Solver->add(z3::implies(
      *OptionToVariableMapping[Feature.getName()],
      *OptionToVariableMapping[Feature.getParentFeature()->getName()]));

  if (!Feature.isOptional()) {
    Solver->add(z3::implies(
        *OptionToVariableMapping[Feature.getParentFeature()->getName()],
        *OptionToVariableMapping[Feature.getName()]));
  }

  SolverConstraintVisitor SCV(this);

  return Ok();
}

Result<SolverErrorCode> Z3Solver::excludeCurrentConfiguration() {
  if (Solver->check() == z3::unsat) {
    return UNSAT;
  }
  z3::model M = Solver->get_model();
  z3::expr Expr = Context.bool_val(false);
  for (auto Option : OptionToVariableMapping.keys()) {
    z3::expr OptionExpr = *OptionToVariableMapping[Option];
    z3::expr Value = M.eval(OptionExpr, true);
    if (Value.is_bool()) {
      if (Value.is_true()) {
        Expr = Expr || !*OptionToVariableMapping[Option];
      } else {
        Expr = Expr || *OptionToVariableMapping[Option];
      }
    } else {
      Expr = Expr || (*OptionToVariableMapping[Option] != Value);
    }
  }
  Solver->add(Expr);
  return Ok();
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getCurrentConfiguration() {
  if (Solver->check() == z3::unsat) {
    return UNSAT;
  }
  z3::model M = Solver->get_model();
  std::unique_ptr<vara::feature::Configuration> Config(
      new vara::feature::Configuration());

  for (auto Option : OptionToVariableMapping.keys()) {
    z3::expr OptionExpr = *OptionToVariableMapping[Option];
    z3::expr Value = M.eval(OptionExpr, true);
    Config->setConfigurationOption(Option, llvm::StringRef(Value.to_string()));
  }
  return Config;
}

// Class SolverConstraintVisitor
bool SolverConstraintVisitor::addConstraint(vara::feature::Constraint *C) {
  if (C->accept(*this)) {
    S->Solver->add(Z3ConstraintExpression);
    return true;
  }
  return false;
}

bool SolverConstraintVisitor::visit(vara::feature::BinaryConstraint *C) {

  bool LHS = C->getLeftOperand()->accept(*this);
  z3::expr Left = Z3ConstraintExpression;
  bool RHS = C->getRightOperand()->accept(*this);
  if (!LHS || !RHS) {
    return false;
  }
  switch (C->getKind()) {
  case feature::Constraint::ConstraintKind::CK_ADDITION:
    Z3ConstraintExpression = Left + Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_AND:
    Z3ConstraintExpression = Left && Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_DIVISION:
    Z3ConstraintExpression = Left / Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_EQUAL:
    Z3ConstraintExpression = Left == Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_EQUIVALENCE:
    Z3ConstraintExpression = z3::implies(Left, Z3ConstraintExpression) &&
                             z3::implies(Z3ConstraintExpression, Left);
    break;
  case feature::Constraint::ConstraintKind::CK_EXCLUDES:
    Z3ConstraintExpression = z3::implies(Left, !Z3ConstraintExpression);
    break;
  case feature::Constraint::ConstraintKind::CK_GREATER:
    Z3ConstraintExpression = Left > Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_GREATER_EQUAL:
    Z3ConstraintExpression = Left >= Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_IMPLIES:
    Z3ConstraintExpression = z3::implies(Left, Z3ConstraintExpression);
    break;
  case feature::Constraint::ConstraintKind::CK_LESS:
    Z3ConstraintExpression = Left < Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_LESS_EQUAL:
    Z3ConstraintExpression = Left <= Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_MULTIPLICATION:
    Z3ConstraintExpression = Left * Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_NOT_EQUAL:
    Z3ConstraintExpression = Left != Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_OR:
    Z3ConstraintExpression = Left || Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_SUBTRACTION:
    Z3ConstraintExpression = Left - Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_XOR:
    Z3ConstraintExpression = z3::implies(Left, !Z3ConstraintExpression) &&
                             z3::implies(Z3ConstraintExpression, !Left);
    break;
  default:
    return false;
  }
  return true;
}

bool SolverConstraintVisitor::visit(vara::feature::UnaryConstraint *C) {
  if (!C->getOperand()->accept(*this)) {
    return false;
  }
  switch (C->getKind()) {
  case feature::Constraint::ConstraintKind::CK_NEG:
    Z3ConstraintExpression = -Z3ConstraintExpression;
    break;
  case feature::Constraint::ConstraintKind::CK_NOT:
    Z3ConstraintExpression = !Z3ConstraintExpression;
    break;
  default:
    return false;
  }
  return true;
}

bool SolverConstraintVisitor::visit(
    vara::feature::PrimaryFeatureConstraint *C) {
  if (C->getFeature()->getKind() ==
      vara::feature::Feature::FeatureKind::FK_NUMERIC) {
    Z3ConstraintExpression =
        S->Context.int_const(C->getFeature()->getName().str().c_str());
  } else {
    Z3ConstraintExpression =
        S->Context.bool_const(C->getFeature()->getName().str().c_str());
  }
  return true;
}

} // namespace vara::solver
