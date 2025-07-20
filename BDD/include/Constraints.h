#ifndef OXIDD_CONSTRAINTS
#define OXIDD_CONSTRAINTS

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/FeatureModel.h"
#include "BDD/include/BDDFactory.h"
#include <unordered_map>

namespace oxidd::capi {

class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
  BDDConstraintVisitor(oxidd_bdd_manager_t manager, 
                      std::unordered_map<std::string, oxidd_bdd_t>& varMap)
      : Manager(manager), VarMap(varMap), CurrentBDD(oxidd_bdd_false(manager)) {}

  oxidd_bdd_t addConstraint(vara::feature::Constraint* C);

  bool visit(vara::feature::BinaryConstraint* C);

  bool visit(vara::feature::UnaryConstraint* C);

  bool visit(vara::feature::PrimaryFeatureConstraint* C);

  bool visit(vara::feature::PrimaryIntegerConstraint* C);

  bool visit(vara::feature::BinaryIntegerConstraint* C);

private:
  bool handleFeatureConstraint(const std::string& featureName);

  oxidd_bdd_manager_t Manager;
  std::unordered_map<std::string, oxidd_bdd_t>& VarMap;
  oxidd_bdd_t CurrentBDD;
};

void processConstraints(
    oxidd_bdd_manager_t manager,
    oxidd_bdd_t& bdd,
    std::unordered_map<std::string, oxidd_bdd_t>& varMap,
    const vara::feature::FeatureModel& model);

} // namespace oxidd::capi

#endif // OXIDD_CONSTRAINTS