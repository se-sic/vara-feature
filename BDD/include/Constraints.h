#ifndef OXIDD_CONSTRAINTS
#define OXIDD_CONSTRAINTS

#include "oxidd/bdd.hpp"
#include "BDDFactory.h"
#include "oxidd/util.hpp"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/FeatureModel.h"
#include <unordered_map>
#include <utility>

namespace bdd::sample {

  using GlobalVarMap = std::unordered_map<oxidd::level_no_t, BDDFactory::BDDFeat>;

  class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
  public:
    BDDConstraintVisitor(oxidd::bdd_manager &Manager,
                        GlobalVarMap &VarMap,
                        oxidd::bdd_function &FinalBDD,
                        bool IsMixedConstraint = false,
                        bool RequireAll = true)
      : Manager(&Manager),
        VarMap(&VarMap),
        CurrentBDD(&FinalBDD),
        IsMixedConstraint(IsMixedConstraint),
        RequireAll(RequireAll),
        ExprBDD(Manager.t()),
        VariableConstraint(Manager.f()) {}

    // Build a BDD for a constraint. If negate==true, we negate the resulting BDD.
    oxidd::bdd_function addConstraint(vara::feature::Constraint* C,
                              bool Negate = false,
                              bool RequireAll = true);

    bool visit(vara::feature::BinaryConstraint* C) override;
    bool visit(vara::feature::UnaryConstraint* C) override;
    bool visit(vara::feature::PrimaryFeatureConstraint* C) override;
    [[nodiscard]] oxidd::bdd_function getExpr() const { return ExprBDD; }

  private:
    // bool isNumericComparison(vara::feature::BinaryConstraint* C);
    // oxidd::bdd_function handleNumericExpr(vara::feature::BinaryConstraint* C, const std::string& op);
    // oxidd::bdd_function createTempVarForOperation(vara::feature::BinaryConstraint* C);
    // oxidd::bdd_function createTempVar(const std::string& name);
    // std::string getOperandName(vara::feature::Constraint* operand);
    bool handleFeatureConstraint(oxidd::var_no_t Id);

    oxidd::bdd_manager *Manager;
    GlobalVarMap *VarMap;

    oxidd::bdd_function *CurrentBDD;
    int         TempCounter = 0;
    bool        IsMixedConstraint = false;
    bool        RequireAll        = true;
    oxidd::bdd_function ExprBDD;
    oxidd::bdd_function VariableConstraint;
 }; 

  // One-pass application of all constraints in the model
  template <typename T>
  bool processConstraints(//TODOoxidd::bdd_manager &Manager,
                          //TODOoxidd::bdd_function &Bdd,
                          //TODOGlobalVarMap &VarMap,
                          //TODOconst vara::feature::FeatureModel &Model,
                          //TODOBDDFactory &Factory
                          BDDConstraintVisitor &Visitor,
                          const T &Constraint,
                          oxidd::bdd_function &Bdd
                          ){
    oxidd::bdd_function ConstraintBDD = Visitor.addConstraint(Constraint->constraint());
    if (ConstraintBDD.is_invalid()) {
          std::cerr << "Warning: Failed to process constraint. Skipping.\n";
          return false;
        }
          Bdd = Bdd & ConstraintBDD;
          return true;
    };    
  
} // namespace bdd::sample

#endif // OXIDD_CONSTRAINTS
