#ifndef OXIDD_CONSTRAINTS
#define OXIDD_CONSTRAINTS

#include "oxidd/bdd.hpp"
#include "oxidd/capi.h"
#include "vara/Feature/FeatureModel.h"
#include "BDDFactory.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <utility>

namespace oxidd::capi {

using GlobalVarMap = std::unordered_map<std::string, BDDFactory::BDDFeat>;
using BinaryVarMap = std::unordered_map<std::string, oxidd_bdd_t>;
using NumericVarMap =
  std::unordered_map<std::string, std::vector<std::pair<std::string, oxidd_bdd_t>>>;

class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
  BDDConstraintVisitor(oxidd_bdd_manager_t manager,
                       GlobalVarMap* varMap,
                       BinaryVarMap* binaryVarMap,
                       NumericVarMap* numericVarMap,
                       bool isMixedConstraint = false,
                       bool requireAll = false)
    : Manager(manager),
      VarMap(varMap),
      BinaryVarMap(binaryVarMap),
      NumericVarMap(numericVarMap),
      CurrentBDD(oxidd_bdd_false(manager)),
      IsMixedConstraint(isMixedConstraint),
      RequireAll(requireAll),
      VariableConstraint(oxidd_bdd_false(manager)) {}

  // Build a BDD for a constraint. If negate==true, we negate the resulting BDD.
  oxidd_bdd_t addConstraint(vara::feature::Constraint* C,
                            bool negate = false,
                            bool requireAll = false);

  bool visit(vara::feature::BinaryConstraint* C) override;
  bool visit(vara::feature::UnaryConstraint* C) override;
  bool visit(vara::feature::PrimaryFeatureConstraint* C) override;
  bool visit(vara::feature::PrimaryIntegerConstraint* C) override;

private:
  bool isNumericComparison(vara::feature::BinaryConstraint* C);
  oxidd_bdd_t handleNumericExpr(vara::feature::BinaryConstraint* C, const std::string& op);
  oxidd_bdd_t createTempVarForOperation(vara::feature::BinaryConstraint* C);
  oxidd_bdd_t createTempVar(const std::string& name);
  std::string getOperandName(vara::feature::Constraint* operand);
  bool handleFeatureConstraint(const std::string& name);

  oxidd_bdd_manager_t Manager;
  GlobalVarMap*  VarMap;
  BinaryVarMap*  BinaryVarMap;
  NumericVarMap* NumericVarMap;

  oxidd_bdd_t CurrentBDD;
  int         tempCounter = 0;
  bool        IsMixedConstraint = false;
  bool        RequireAll        = false;
  oxidd_bdd_t VariableConstraint;
};

// One-pass application of all constraints in the model
void processConstraints(oxidd_bdd_manager_t manager,
                        oxidd_bdd_t& bdd,
                        GlobalVarMap& varMap,
                        BinaryVarMap& binaryVarMap,
                        NumericVarMap& numericVarMap,
                        const vara::feature::FeatureModel& model);

} // namespace oxidd::capi

#endif // OXIDD_CONSTRAINTS
