#include "vara/Feature/FeatureModel.h"
#include "BDD/include/BDDFactory.h"
#include <unordered_map>
#include "BDD/include/Cosnstraints.h"

namespace oxidd::capi {

class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
  BDDConstraintVisitor(oxidd_bdd_manager_t manager, 
                      std::unordered_map<std::string, oxidd_bdd_t>* varMap)
      : Manager(manager), VarMap(*varMap), CurrentBDD(oxidd_bdd_false(manager)) {}

  oxidd_bdd_t addConstraint(vara::feature::Constraint* C) {
    C->accept(this);
    return CurrentBDD;
  }

  bool visit(vara::feature::BinaryConstraint* C) override {
    C->getLeft()->accept(*this);
    oxidd_bdd_t left = CurrentBDD;
    
    C->getRight()->accept(this);
    oxidd_bdd_t right = CurrentBDD;

    switch (C->getKind()) {
      case vara::feature::BinaryConstraint::AND:
        CurrentBDD = oxidd_bdd_and(left, right);
        break;
      case vara::feature::BinaryConstraint::OR:
        CurrentBDD = oxidd_bdd_or(left, right);
        break;
      case vara::feature::BinaryConstraint::IMPLIES:
        CurrentBDD = oxidd_bdd_imp(left, right);
        break;
      case vara::feature::BinaryConstraint::EQUIVALENT:
        CurrentBDD = oxidd_bdd_equiv(left, right);
        break;
      default:
        CurrentBDD = oxidd_bdd_false(Manager);
        break;
    }
    return true;
  }

  bool visit(vara::feature::UnaryConstraint* C) override {
    C->getOperand()->accept(this);
    
    if (C->getKind() == vara::feature::UnaryConstraint::NOT) {
      CurrentBDD = oxidd_bdd_not(CurrentBDD);
    } else {
      CurrentBDD = oxidd_bdd_false(Manager);
    }
    return true;
  }

  bool visit(vara::feature::PrimaryFeatureConstraint* C) override {
    return handleFeatureConstraint(C->getFeatureName().str());
  }

  bool visit(vara::feature::PrimaryIntegerConstraint* C) override {
    return handleFeatureConstraint(C->getFeatureName().str());
  }

  bool visit(vara::feature::BinaryIntegerConstraint* C) override {
    std::string constraintName = C->toString();
    return handleFeatureConstraint(constraintName);
  }

private:
  bool handleFeatureConstraint(const std::string& featureName) {
    auto it = VarMap.find(featureName);
    if (it != VarMap.end()) {
      CurrentBDD = it->second;
    } else {
      oxidd_bdd_t var = oxidd_bdd_new_var(Manager);
      VarMap[featureName] = var;
      CurrentBDD = var;
    }
    return true;
  }

  oxidd_bdd_manager_t Manager;
  std::unordered_map<std::string, oxidd_bdd_t>& VarMap;
  oxidd_bdd_t CurrentBDD;
};

void processConstraints(
    oxidd_bdd_manager_t manager,
    oxidd_bdd_t& bdd,
    std::unordered_map<std::string, oxidd_bdd_t>& varMap,
    const vara::feature::FeatureModel& model) {
  
  BDDConstraintVisitor visitor(manager, varMap);

  for (const auto& C : model.booleanConstraints()) {
    oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
    bdd = oxidd_bdd_and(bdd, constraintBDD);
  }

  for (const auto& C : model.nonBooleanConstraints()) {
    oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
    bdd = oxidd_bdd_and(bdd, constraintBDD);
  }

  for (const auto& C : model.mixedConstraints()) {
    oxidd_bdd_t constraintBDD = visitor.addConstraint(C->constraint());
    bdd = oxidd_bdd_and(bdd, constraintBDD);
  }
}

} // namespace oxidd::capi