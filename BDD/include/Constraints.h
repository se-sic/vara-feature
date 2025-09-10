#ifndef OXIDD_CONSTRAINTS
#define OXIDD_CONSTRAINTS

#include "oxidd/capi.h"
#include "BDDFactory.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include <algorithm>
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

namespace oxidd::capi {

  using GlobalVarMap = std::unordered_map<oxidd_var_no_t, BDDFactory::BDDFeat>;

  class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
  public:
    BDDConstraintVisitor(oxidd_bdd_manager_t manager,
                        GlobalVarMap* varMap,
                        oxidd_bdd_t finalBDD,
                        bool isMixedConstraint = false,
                        bool requireAll = true)
      : Manager(manager),
        VarMap(varMap),
        CurrentBDD(finalBDD),
        IsMixedConstraint(isMixedConstraint),
        RequireAll(requireAll),
        VariableConstraint(oxidd_bdd_false(manager)) {}

    // Build a BDD for a constraint. If negate==true, we negate the resulting BDD.
    oxidd_bdd_t addConstraint(vara::feature::Constraint* C,
                              bool negate = false,
                              bool requireAll = true);

    bool visit(vara::feature::BinaryConstraint* C) override;
    bool visit(vara::feature::UnaryConstraint* C) override;
    bool visit(vara::feature::PrimaryFeatureConstraint* C) override;

  private:
    // bool isNumericComparison(vara::feature::BinaryConstraint* C);
    // oxidd_bdd_t handleNumericExpr(vara::feature::BinaryConstraint* C, const std::string& op);
    // oxidd_bdd_t createTempVarForOperation(vara::feature::BinaryConstraint* C);
    // oxidd_bdd_t createTempVar(const std::string& name);
    // std::string getOperandName(vara::feature::Constraint* operand);
    bool handleFeatureConstraint(const oxidd_var_no_t id);

    oxidd_bdd_manager_t Manager;
    GlobalVarMap*  VarMap;

    oxidd_bdd_t CurrentBDD;
    int         tempCounter = 0;
    bool        IsMixedConstraint = false;
    bool        RequireAll        = true;
    oxidd_bdd_t VariableConstraint;
 }; 

  // One-pass application of all constraints in the model
  void processConstraints(oxidd_bdd_manager_t manager,
                          oxidd_bdd_t& bdd,
                          GlobalVarMap& varMap,
                          const vara::feature::FeatureModel& model);
}// namespace oxidd::capi

#endif // OXIDD_CONSTRAINTS
