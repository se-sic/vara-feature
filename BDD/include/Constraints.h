#ifndef OXIDD_CONSTRAINTS
#define OXIDD_CONSTRAINTS

#include "vara/Feature/Constraint.h"
#include "BDDFactory.h"
#include "oxidd/bdd.hpp"
#include "oxidd/util.hpp"
#include <iostream>

namespace bdd::sample {

  /// \brief Visitor that turns vara::feature::Cosntraint into a BDD.
  ///
  /// The caller (BDDFactory.cpp) constructs a visitor with a manager and target BDD
  /// and calls \p processConstraints() for each constraint in the feature model. The 
  /// results are then AND-ed into the target BDD.
  ///
  /// \note Currently only considers binary and unary constrains; any other type 
  // causes the visitor to return false.
  class BDDConstraintVisitor : public vara::feature::ConstraintVisitor {
  public:
    /// \brief Constructor for the Visitor that ANDs the built BDD into \p FinalBDD.
    ///
    /// \param Manager The manager used for the BDD
    /// \param FinalBDD The target BDD which is returned in the end
    /// \param IsMixedConstraint  Fallback guard in case of mixed constraints, e.g. binary
    ///                           and numeric contraints. Uses \p VariableConstraint as an accumulator.
    ///                           Currently not executed in our case.
    /// \param RequireAll Only meaningful in combination with \p IsMixedConstraint.
    ///                   Currently not executed in our case, a default value is given.
    BDDConstraintVisitor(oxidd::bdd_manager &Manager,
                        oxidd::bdd_function &FinalBDD,
                        bool IsMixedConstraint = false,
                        bool RequireAll = true)
      : Manager(&Manager),
        CurrentBDD(&FinalBDD),
        IsMixedConstraint(IsMixedConstraint),
        RequireAll(RequireAll),
        ExprBDD(Manager.t()),
        VariableConstraint(Manager.f()) {}

    /// \brief Builds a BDD for \p C and ANDs it into the target BDD.
    /// 
    /// Resets all accumulators, visist \p C and on success ANDs it as a
    /// a standalone BDD into the target BDD.
    ///
    /// \param C Constraint to be processed
    /// \param Negate If true, we negate the reulting BDD
    /// \param RequireAll See constructor doc
    ///
    /// \return The standalone BDD for \p C if true or a false BDD
    /// if processing was not successful. In latter case, the target BDD
    /// remaind unchanged.
    oxidd::bdd_function addConstraint(vara::feature::Constraint* C,
                              bool Negate = false,
                              bool RequireAll = true);

    /// \brief Visits a binary constraint and handles AND, OR, IMPLIES, EQUIVALENCE,
    /// XOR, EXCLUDES, EQUAL, NOT_EQUAL by recursing on both operands and then combining them 
    /// with the corresponding operation.
    bool visit(vara::feature::BinaryConstraint* C) override;

    /// \brief Visits an unary constraint and handles NOT by recursing on the operand and
    /// negating the result.
    bool visit(vara::feature::UnaryConstraint* C) override;

    /// \brief Visits a primary feature constraint aka a single operand and looks up the
    /// BDD variable and stores it temporarily in \p ExprBDD. If the feature is numeric, we return a false BDD.
    bool visit(vara::feature::PrimaryFeatureConstraint* C) override;

    /// \brief Returns the BDD constructed by the current expression.
    [[nodiscard]] oxidd::bdd_function getExpr() const { return ExprBDD; }

    private:
      oxidd::bdd_manager *Manager;
      oxidd::bdd_function *CurrentBDD;
      bool        IsMixedConstraint = false;
      bool        RequireAll        = true;
      oxidd::bdd_function ExprBDD;
      oxidd::bdd_function VariableConstraint;
  };

  /// \brief Extracts the constraint from \p Constraint and build its BDD through \p Visitor:
  /// The result is AND-ed into \p BDD.
  ///
  /// \param Visitor The visitor that constructs a BDD based on the constraint 
  /// \param Constraint Holds the constraint to be processed
  /// \param Bdd The tahet BDD which is AND-ed with the constraint BDD if the visit was successful.
  ///            Else is reamins unchanged.
  template <typename T>
  bool processConstraints(BDDConstraintVisitor &Visitor,
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
  }
  
} // namespace bdd::sample

#endif // OXIDD_CONSTRAINTS
