#include "BDDFactory.h"
#include "vara/Feature/FeatureModel.h"
#include <algorithm>
#include <unordered_map>
#include "Constraints.h"


using GlobalVarMap = std::unordered_map<std::string, oxidd::capi::BDDFactory::BDDFeat>;
using BinaryVarMap = std::unordered_map<std::string, oxidd::capi::oxidd_bdd_t>;
using NumericVarMap = std::unordered_map<std::string, std::vector<std::pair<std::string, oxidd::capi::oxidd_bdd_t>>>;

namespace oxidd::capi {

  oxidd_bdd_t BDDConstraintVisitor::addConstraint(vara::feature::Constraint* C, bool negate, bool requireAll) {
    this->RequireAll = requireAll;
    C->accept(*this);
    if (negate) {
      CurrentBDD = oxidd_bdd_or(oxidd_bdd_false(Manager), CurrentBDD); //TODO --> DONE?
    }
    if (IsMixedConstraint && RequireAll) {
      CurrentBDD = oxidd_bdd_or(VariableConstraint, CurrentBDD);
    } 
    return CurrentBDD;
  }

  bool BDDConstraintVisitor::visit(vara::feature::BinaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    
    C->getLeftOperand()->accept(*this);
    oxidd_bdd_t left = CurrentBDD;
    
    C->getRightOperand()->accept(*this);
    oxidd_bdd_t right = CurrentBDD;

    switch(C->getKind()) {
      case CK::CK_AND:
        CurrentBDD = oxidd_bdd_and(left, right);
        break;
      case CK::CK_OR:
        CurrentBDD = oxidd_bdd_or(left, right);
        break;
      case CK::CK_IMPLIES:
        CurrentBDD = oxidd_bdd_imp(left, right);
        break;
      case CK::CK_EQUIVALENCE:
        CurrentBDD = oxidd_bdd_equiv(left, right);
        break;
      case CK::CK_XOR:
        CurrentBDD = oxidd_bdd_xor(left, right);
        break;
      case CK::CK_EXCLUDES:
        CurrentBDD = oxidd_bdd_imp(left, oxidd_bdd_not(right));
        break;

      case CK::CK_LESS:
        CurrentBDD = handleNumericExpr(C, "<");
        break;
      case CK::CK_GREATER:
        CurrentBDD = handleNumericExpr(C, ">");
        break;
      case CK::CK_LESS_EQUAL:
        CurrentBDD = handleNumericExpr(C, "<=");
        break;
      case CK::CK_GREATER_EQUAL:
        CurrentBDD = handleNumericExpr(C, ">=");
        break;
      case CK::CK_EQUAL:
        if (isNumericComparison(C)) {
          CurrentBDD = handleNumericExpr(C, "=");
        } else {
          CurrentBDD = oxidd_bdd_equiv(left, right);
        }
        break;
      case CK::CK_NOT_EQUAL:
        if (isNumericComparison(C)) {
          CurrentBDD = handleNumericExpr(C, "!=");
        } else {
          CurrentBDD = oxidd_bdd_not(oxidd_bdd_equiv(left, right));
        }
        break;

      case CK::CK_ADDITION:
      case CK::CK_SUBTRACTION:
      case CK::CK_MULTIPLICATION:
      case CK::CK_DIVISION:
        CurrentBDD = createTempVarForOperation(C);
        break;

      default:
        CurrentBDD = oxidd_bdd_false(Manager);
        return false;
    }
    return true;
  }

  bool BDDConstraintVisitor::visit(vara::feature::UnaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    
    C->getOperand()->accept(*this);
    switch(C->getKind()) {
      case CK::CK_NOT:
        CurrentBDD = oxidd_bdd_not(CurrentBDD);
        return true;
      case CK::CK_NEG:
        CurrentBDD = createTempVar("neg");
        return true;
      default:
        CurrentBDD = oxidd_bdd_false(Manager);
        return false;
    }
  }

  bool BDDConstraintVisitor::visit(vara::feature::PrimaryFeatureConstraint* C) {
    std::string featureName = C->getFeature()->getName().str();

    if (C->getFeature()->getKind()== vara::feature::Feature::FeatureKind::FK_NUMERIC) {
      if (IsMixedConstraint) {
        oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
        (*BinaryVarMap) [featureName] = var;
        VariableConstraint = oxidd_bdd_or(VariableConstraint, oxidd_bdd_not(var));
        CurrentBDD = var;
        return true;
      } else {
        return handleFeatureConstraint(featureName);
      }
    } else {
      return handleFeatureConstraint(featureName);
    }
    
  }

  bool BDDConstraintVisitor::visit(vara::feature::PrimaryIntegerConstraint* C) {
    CurrentBDD = oxidd_bdd_false(Manager);
    return true;
  }


  bool BDDConstraintVisitor::isNumericComparison(vara::feature::BinaryConstraint* C) {
    using namespace vara::feature;
    return (dynamic_cast<NumericConstraint*>(C->getLeftOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getLeftOperand())) &&
           (dynamic_cast<NumericConstraint*>(C->getRightOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getRightOperand()));
  }

  oxidd_bdd_t BDDConstraintVisitor::handleNumericExpr(vara::feature::BinaryConstraint* C, 
                              const std::string& op) {
    std::string left = getOperandName(C->getLeftOperand());
    std::string right = getOperandName(C->getRightOperand());
    std::string constraintName = left + op + right;
    
    // Check if we already have this constraint
    auto it = VarMap->find(constraintName);
    oxidd_bdd_t child;
    if (it != VarMap->end()) {
      auto feat = it->second;  //TODO: prolly wrong --> DONE?
      if(feat.type == BDDFactory::featType::BINARY) {
        oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
        CurrentBDD =  *node_ptr;
      } else if(feat.type == BDDFactory::featType::NUMERIC) {
        auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(feat.data);
         child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
            return pair.first == constraintName;}
          )->second;
        if (child._p != nullptr) {
          CurrentBDD = child;
        }
      }
      return oxidd_bdd_t{nullptr, 0};
    }


    // Create new variable for this constraint
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*BinaryVarMap)[constraintName] = var;
    return var;
  }

  oxidd_bdd_t BDDConstraintVisitor::createTempVarForOperation(vara::feature::BinaryConstraint* C) {
    using CK = vara::feature::Constraint::ConstraintKind;
    static const std::unordered_map<CK, std::string> opMap = {
      {CK::CK_ADDITION, "add"},
      {CK::CK_SUBTRACTION, "sub"},
      {CK::CK_MULTIPLICATION, "mul"},
      {CK::CK_DIVISION, "div"}
    };
    
    std::string tempName = opMap.at(C->getKind()) + "_" + std::to_string(tempCounter++);
    return createTempVar(tempName);
  }

  oxidd_bdd_t BDDConstraintVisitor::createTempVar(const std::string& name) {
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*BinaryVarMap)[name] = var;
    return var;
  }

  std::string BDDConstraintVisitor::getOperandName(vara::feature::Constraint* operand) {
    using namespace vara::feature;
    
    if (auto fc = dynamic_cast<PrimaryFeatureConstraint*>(operand)) {
      return fc->getFeature()->getName().str();
    }
    else if (auto ic = dynamic_cast<PrimaryIntegerConstraint*>(operand)) {
      return std::to_string(ic->getValue());
    }
    return "const_0";
  }

  bool BDDConstraintVisitor::handleFeatureConstraint(const std::string& name) {
    auto it = VarMap->find(name);
    oxidd_bdd_t child;
    if (it != VarMap->end()) { //TODO --> DONE?
      auto feat = it->second;
      if(feat.type == BDDFactory::featType::BINARY) {
        oxidd_bdd_t* node_ptr = std::get<oxidd_bdd_t*>(feat.data);
        CurrentBDD = *node_ptr;
      } else if(feat.type == BDDFactory::featType::NUMERIC) {
        auto numericFeats = std::get<std::vector<std::pair<string, oxidd_bdd_t>>*>(feat.data);
          child = std::find_if(numericFeats->begin(), numericFeats->end(), [&](const std::pair<std::string, oxidd_bdd_t>& pair){
            return pair.first == name;}
          )->second;
          if (child._p != nullptr) {
          CurrentBDD = child;
        }
      }
      return true;
    }
    CurrentBDD = createTempVar(name);
    return true;
  }

void processConstraints(
    oxidd_bdd_manager_t manager,
    oxidd_bdd_t& bdd,
    GlobalVarMap& varMap,
    ::BinaryVarMap& binaryVarMap,
    ::NumericVarMap& numericVarMap,
    const vara::feature::FeatureModel& model) {
  
  BDDConstraintVisitor visitor(manager, &varMap, &binaryVarMap, &numericVarMap);

  const auto process = [&](const auto& constraint) {
    oxidd_bdd_t constraintBDD = visitor.addConstraint(constraint->constraint());
    if (constraintBDD._p == nullptr) return false;
    bdd = oxidd_bdd_and(bdd, constraintBDD);
    return true;
  };

  const auto processMixed = [&] (const auto& constraint) {
    BDDConstraintVisitor mixedVisitor(manager, &varMap, &binaryVarMap, &numericVarMap, true);
    oxidd_bdd_t constraintBDD = mixedVisitor.addConstraint(constraint->constraint(), 
      constraint->exprKind() == vara::feature::FeatureModel::MixedConstraint::ExprKind::NEG,
      constraint->req() == vara::feature::FeatureModel::MixedConstraint::Req::ALL);
    if (constraintBDD._p == nullptr) {
      return false;
    }
    bdd = oxidd_bdd_and(bdd, constraintBDD);
    return true;
  };

  for (const auto& C : model.booleanConstraints()) if (!process(C)) break;
  for (const auto& C : model.nonBooleanConstraints()) if (!process(C)) break;
  for (const auto& C : model.mixedConstraints()) if (!process(C)) break;
}

} // namespace oxidd::capi