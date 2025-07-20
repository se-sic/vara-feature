#include "BDD/include/BDDFactory.h"
#include "vara/Feature/FeatureModel.h"
#include <algorithm>
#include <unordered_map>
#include "BDD/include/Constraints.h"

namespace oxidd::capi {

using GlobalVarMap = std::unordered_map<std::string, oxidd_bdd_t>;
using BinaryVarMap = std::unordered_map<std::string, oxidd_bdd_t>;
using NumericVarMap = std::unordered_map<std::string, std::vector<std::pair<std::string, oxidd_bdd_t>>>;

class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
<<<<<<< HEAD
  BDDConstraintVisitor(oxidd_bdd_manager_t manager, 
                      std::unordered_map<std::string, oxidd_bdd_t>* varMap)
      : Manager(manager), VarMap(*varMap), CurrentBDD(oxidd_bdd_false(manager)) {}
=======
  BDDConstraintVisitor(oxidd_bdd_manager_t manager,
                      GlobalVarMap* varMap,
                      BinaryVarMap* binaryVarMap,
                      NumericVarMap* numericVarMap)
      : Manager(manager), VarMap(varMap), 
        BinaryVarMap(binaryVarMap), NumericVarMap(numericVarMap),
        CurrentBDD(oxidd_bdd_false(manager)) {}
>>>>>>> f56458bb (Changed Constraints.cpp)

  oxidd_bdd_t addConstraint(vara::feature::Constraint* C) {
    C->accept(*this);
    return CurrentBDD;
  }

  bool visit(vara::feature::BinaryConstraint* C) override {
<<<<<<< HEAD
    C->getLeft()->accept(*this);
=======
    using CK = vara::feature::Constraint::ConstraintKind;
    
    C->getLeftOperand()->accept(*this);
>>>>>>> f56458bb (Changed Constraints.cpp)
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

  bool visit(vara::feature::UnaryConstraint* C) override {
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

  bool visit(vara::feature::PrimaryFeatureConstraint* C) override {
    std::string featureName = C->getFeature()->getName().str();
    return handleFeatureConstraint(featureName);
  }

  bool visit(vara::feature::PrimaryIntegerConstraint* C) override {
    CurrentBDD = oxidd_bdd_false(Manager);
    return true;
  }

private:
  bool isNumericComparison(vara::feature::BinaryConstraint* C) {
    using namespace vara::feature;
    return (dynamic_cast<NumericConstraint*>(C->getLeftOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getLeftOperand())) &&
           (dynamic_cast<NumericConstraint*>(C->getRightOperand()) ||
            dynamic_cast<PrimaryIntegerConstraint*>(C->getRightOperand()));
  }

  oxidd_bdd_t handleNumericExpr(vara::feature::BinaryConstraint* C, 
                              const std::string& op) {
    std::string left = getOperandName(C->getLeftOperand());
    std::string right = getOperandName(C->getRightOperand());
    std::string constraintName = left + op + right;
    
    // Check if we already have this constraint
    auto it = VarMap->find(constraintName);
    if (it != VarMap->end()) {
      return it->second;
    }
    
    // Create new variable for this constraint
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*BinaryVarMap)[constraintName] = var;
    (*VarMap)[constraintName] = var;
    return var;
  }

  oxidd_bdd_t createTempVarForOperation(vara::feature::BinaryConstraint* C) {
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

  oxidd_bdd_t createTempVar(const std::string& name) {
    oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
    (*BinaryVarMap)[name] = var;
    (*VarMap)[name] = var;
    return var;
  }

  std::string getOperandName(vara::feature::Constraint* operand) {
    using namespace vara::feature;
    
    if (auto fc = dynamic_cast<PrimaryFeatureConstraint*>(operand)) {
      return fc->getFeature()->getName().str();
    }
    else if (auto ic = dynamic_cast<PrimaryIntegerConstraint*>(operand)) {
      return std::to_string(ic->getValue());
    }
    return "const_0";
  }

  bool handleFeatureConstraint(const std::string& name) {
    auto it = VarMap->find(name);
    if (it != VarMap->end()) {
      CurrentBDD = it->second;
      return true;
    }
    
    CurrentBDD = createTempVar(name);
    return true;
  }

  oxidd_bdd_manager_t Manager;
  GlobalVarMap* VarMap;
  BinaryVarMap* BinaryVarMap;
  NumericVarMap* NumericVarMap;
  oxidd_bdd_t CurrentBDD;
  int tempCounter = 0;
};

void processConstraints(
    oxidd_bdd_manager_t manager,
    oxidd_bdd_t& bdd,
    GlobalVarMap& varMap,
    BinaryVarMap& binaryVarMap,
    NumericVarMap& numericVarMap,
    const vara::feature::FeatureModel& model) {
  
  BDDConstraintVisitor visitor(manager, &varMap, &binaryVarMap, &numericVarMap);

  const auto process = [&](const auto& constraint) {
    oxidd_bdd_t constraintBDD = visitor.addConstraint(constraint->constraint());
    if (constraintBDD._p == nullptr) return false;
    bdd = oxidd_bdd_and(bdd, constraintBDD);
    return true;
  };

  for (const auto& C : model.booleanConstraints()) if (!process(C)) break;
  for (const auto& C : model.nonBooleanConstraints()) if (!process(C)) break;
  for (const auto& C : model.mixedConstraints()) if (!process(C)) break;
}

} // namespace oxidd::capi