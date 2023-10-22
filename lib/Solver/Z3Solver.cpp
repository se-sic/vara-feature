#include "vara/Solver/Z3Solver.h"

#include "llvm/Support/ErrorHandling.h"

#include "z3++.h"

namespace vara::solver {

Result<SolverErrorCode>
Z3Solver::addFeature(const feature::Feature &FeatureToAdd,
                     bool IsInAlternativeGroup) {
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
      auto Range =
          std::get<vara::feature::NumericFeature::ValueRangeType>(Values);
      auto *StepFunction = F->getStepFunction();
      auto Step = Range.first;
      std::vector<int64_t> Vals;
      while (Step <= Range.second) {
        Vals.insert(Vals.begin(), Step);
        Step = StepFunction->next(Step);
      }
      if (auto R = addFeature(F->getName().str(), Vals); !R) {
        return R;
      }
    }
    break;
  }
  case feature::Feature::FeatureKind::FK_BINARY: {
    // Add all constraints (i.e., implications, exclusions, parent feature)
    if (!llvm::isa<vara::feature::BinaryFeature>(&FeatureToAdd)) {
      return NOT_SUPPORTED;
    }
    addFeature(FeatureToAdd.getName().str());
    auto R = setBinaryFeatureConstraints(
        *llvm::dyn_cast<vara::feature::BinaryFeature>(&FeatureToAdd),
        IsInAlternativeGroup);
    if (!R) {
      return R;
    }
    break;
  }
  case feature::Feature::FeatureKind::FK_ROOT:
    // Add root as a constraint; root is mandatory
    addFeature(FeatureToAdd.getName().str());
    assert(OptionToVariableMapping.find(FeatureToAdd.getName()) !=
               OptionToVariableMapping.end() &&
           "No OptionToVariableMapping was defined for the given feature.");
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
  const z3::expr Feature = Context.bool_const(FeatureName.c_str());
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
  const z3::expr Feature = Context.int_const(FeatureName.c_str());
  OptionToVariableMapping.insert(
      std::make_pair(FeatureName, std::make_unique<z3::expr>(Feature)));

  // Add the numeric values as constraints
  z3::expr Constraint = Context.bool_val(false);
  for (int64_t const Value : Values) {
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
  const auto *Parent =
      reinterpret_cast<const feature::Feature *>(R.getParent());
  auto ParentOption = Context.bool_const(Parent->getName().str().c_str());
  z3::expr_vector V(Context);
  for (const auto &Child : R.children()) {
    const auto *ChildFeature =
        reinterpret_cast<const feature::Feature *>(Child);
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
  Z3SolverConstraintVisitor SCV(this);
  const bool Succ = SCV.addConstraint(&ConstraintToAdd);
  if (!Succ) {
    return SolverErrorCode::NOT_SUPPORTED;
  }
  return Ok();
}

Result<SolverErrorCode> Z3Solver::addMixedConstraint(
    feature::Constraint &ConstraintToAdd,
    feature::FeatureModel::MixedConstraint::ExprKind ExprKind,
    feature::FeatureModel::MixedConstraint::Req Req) {
  Z3SolverConstraintVisitor SCV(this, true);
  const bool Succ = SCV.addConstraint(
      &ConstraintToAdd,
      ExprKind == feature::FeatureModel::MixedConstraint::ExprKind::NEG,
      Req == feature::FeatureModel::MixedConstraint::Req::ALL);
  if (!Succ) {
    return SolverErrorCode::NOT_SUPPORTED;
  }
  return Ok();
}

Result<SolverErrorCode, bool> Z3Solver::hasValidConfigurations() {
  if (Solver->check() == z3::sat) {
    return Ok(true);
  }
  return Ok(false);
}

Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
Z3Solver::getNextConfiguration() {
  auto CurrentConfig = getCurrentConfiguration();

  // Add current configuration as a constraint to exclude it for further queries
  excludeCurrentConfiguration();

  // Retrieve the next configuration
  return CurrentConfig;
}

Result<SolverErrorCode, uint64_t> Z3Solver::getNumberValidConfigurations() {
  Solver->push();
  uint64_t Count = 0;
  while (Solver->check() == z3::sat) {
    excludeCurrentConfiguration();
    Count++;
  }
  Solver->pop();
  return Count;
}

Result<SolverErrorCode,
       std::vector<std::unique_ptr<vara::feature::Configuration>>>
Z3Solver::getAllValidConfigurations() {
  Solver->push();
  auto Vector = std::vector<std::unique_ptr<vara::feature::Configuration>>();
  while (Solver->check() == z3::sat) {
    auto Config = getCurrentConfiguration().extractValue();
    Vector.insert(Vector.begin(), std::move(Config));
    excludeCurrentConfiguration();
  }
  Solver->pop();
  return Vector;
}

Result<SolverErrorCode>
Z3Solver::setBinaryFeatureConstraints(const feature::BinaryFeature &Feature,
                                      bool IsInAlternativeGroup) {
  // Add constraint to parent
  Solver->add(z3::implies(
      *OptionToVariableMapping[Feature.getName()],
      *OptionToVariableMapping[Feature.getParentFeature()->getName()]));

  if (!IsInAlternativeGroup && !Feature.isOptional()) {
    Solver->add(z3::implies(
        *OptionToVariableMapping[Feature.getParentFeature()->getName()],
        *OptionToVariableMapping[Feature.getName()]));
  }

  return Ok();
}

Result<SolverErrorCode> Z3Solver::excludeCurrentConfiguration() {
  if (Solver->check() == z3::unsat) {
    return UNSAT;
  }
  const z3::model M = Solver->get_model();
  z3::expr Expr = Context.bool_val(false);
  for (auto Iterator = OptionToVariableMapping.begin();
       Iterator != OptionToVariableMapping.end(); Iterator++) {
    const z3::expr OptionExpr = *Iterator->getValue();
    const z3::expr Value = M.eval(OptionExpr, true);
    if (Value.is_bool()) {
      if (Value.is_true()) {
        Expr = Expr || !*Iterator->getValue();
      } else {
        Expr = Expr || *Iterator->getValue();
      }
    } else {
      Expr = Expr || (*Iterator->getValue() != Value);
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
  const z3::model M = Solver->get_model();
  auto Config = std::make_unique<vara::feature::Configuration>();

  for (auto Iterator = OptionToVariableMapping.begin();
       Iterator != OptionToVariableMapping.end(); Iterator++) {
    const z3::expr OptionExpr = *Iterator->getValue();
    const z3::expr Value = M.eval(OptionExpr, true);
    Config->setConfigurationOption(Iterator->getKey(),
                                   llvm::StringRef(Value.to_string()));
  }
  return Config;
}

// Class Z3SolverConstraintVisitor
bool Z3SolverConstraintVisitor::addConstraint(vara::feature::Constraint *C,
                                              bool NegateExpr,
                                              bool RequireAll) {
  if (C->accept(*this)) {
    if (NegateExpr) {
      Z3ConstraintExpression = !Z3ConstraintExpression;
    }
    if (MixedConstraint && RequireAll) {
      S->Solver->add(VariableConstraint || Z3ConstraintExpression);
    } else {
      S->Solver->add(Z3ConstraintExpression);
    }
    return true;
  }
  return false;
}

bool Z3SolverConstraintVisitor::visit(vara::feature::BinaryConstraint *C) {

  const bool LHS = C->getLeftOperand()->accept(*this);
  const z3::expr Left = Z3ConstraintExpression;
  const bool RHS = C->getRightOperand()->accept(*this);
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
    llvm_unreachable(
        "Unimplemented binary constraint in Z3SolverConstraintVisitor "
        "detected!");
    return false;
  }
  return true;
}

bool Z3SolverConstraintVisitor::visit(vara::feature::UnaryConstraint *C) {
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

bool Z3SolverConstraintVisitor::visit(
    vara::feature::PrimaryFeatureConstraint *C) {
  if (C->getFeature()->getKind() ==
      vara::feature::Feature::FeatureKind::FK_NUMERIC) {
    Z3ConstraintExpression =
        S->Context.int_const(C->getFeature()->getName().str().c_str());
  } else {
    Z3ConstraintExpression =
        S->Context.bool_const(C->getFeature()->getName().str().c_str());
    if (MixedConstraint) {
      VariableConstraint = VariableConstraint || !Z3ConstraintExpression;
      Z3ConstraintExpression = z3::to_expr(
          S->Context, Z3_mk_ite(S->Context, Z3ConstraintExpression,
                                S->Context.int_val(1), S->Context.int_val(0)));
    }
  }
  return true;
}

bool Z3SolverConstraintVisitor::visit(
    vara::feature::PrimaryIntegerConstraint *C) {
  Z3ConstraintExpression = S->Context.int_val(C->getValue());
  return true;
}

} // namespace vara::solver
