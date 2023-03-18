#ifndef VARA_SOLVER_Z3SOLVER_H_
#define VARA_SOLVER_Z3SOLVER_H_

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Solver/Solver.h"
#include "vara/Utils/Result.h"

#include "vara/Feature/Relationship.h"
#include "z3++.h"

namespace vara::solver {
//===----------------------------------------------------------------------===//
//                               Z3Solver Class
//===----------------------------------------------------------------------===//

/// \brief The Z3 solver implementation for handling constraints
class Z3Solver : public Solver {
public:
  explicit Z3Solver(z3::config &Cfg) : Context(Cfg) {
    Solver = std::make_unique<z3::solver>(Context);
  }

  static std::unique_ptr<Z3Solver> create() {
    // The configuration of z3
    z3::config Cfg;
    Cfg.set("model", "true");
    Cfg.set("model-validate", "true");
    return std::make_unique<Z3Solver>(Cfg);
  }

  Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd,
             bool IsInAlternativeGroup = false) override;

  Result<SolverErrorCode> addFeature(const string &FeatureName) override;

  Result<SolverErrorCode>
  addFeature(const string &FeatureName,
             const std::vector<int64_t> &Values) override;

  Result<SolverErrorCode>
  removeFeature(feature::Feature &FeatureToRemove) override;

  Result<SolverErrorCode>
  addRelationship(const feature::Relationship &R) override;

  Result<SolverErrorCode>
  addConstraint(feature::Constraint &ConstraintToAdd) override;

  Result<SolverErrorCode>
  addMixedConstraint(feature::Constraint &ConstraintToAdd,
                     feature::FeatureModel::MixedConstraint::ExprKind ExprKind,
                     feature::FeatureModel::MixedConstraint::Req Req) override;

  Result<SolverErrorCode, bool> hasValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getCurrentConfiguration() override;

  Result<SolverErrorCode, std::unique_ptr<uint64_t>>
  getNumberValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() override;

  Result<SolverErrorCode,
         std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getAllValidConfigurations() override;

private:
  // The Z3SolverConstraintVisitor is a friend class to access the solver and
  // the context.
  friend class Z3SolverConstraintVisitor;

  /// Exclude the current configuration by adding it as a constraint
  /// \return an error code in case of error.
  Result<SolverErrorCode> excludeCurrentConfiguration();

  /// Processes the constraints of the binary feature and ignores the 'optional'
  /// constraint if the feature is in an alternative group.
  /// \return an error code in case of error.
  Result<SolverErrorCode>
  setBinaryFeatureConstraints(const feature::BinaryFeature &Feature,
                              bool IsInAlternativeGroup);

  /// The context of Z3 needed to initialize variables.
  z3::context Context;

  /// This map contains the original feature names as key and maps it to the
  /// Z3 value.
  llvm::StringMap<std::unique_ptr<z3::expr>> OptionToVariableMapping;

  /// The instance of the Z3 solver needed for caching the constraints and
  /// variables.
  std::unique_ptr<z3::solver> Solver;
};

/// \brief This class is a visitor to convert the constraints from the
/// feature model into constraints for Z3.
class Z3SolverConstraintVisitor : public vara::feature::ConstraintVisitor {
public:
  /// Constructs a new solver constraint visitor by using the reference to
  /// the solver. This reference is needed to retrieve the context and reuse
  /// it in the visitor.
  ///
  /// \param S The Z3Solver instance that uses this visitor.
  /// \param MixedConstraint whether the visitor is applied on mixed constraints
  /// or not.
  Z3SolverConstraintVisitor(Z3Solver *S, bool MixedConstraint = false)
      : Z3ConstraintExpression(S->Context),
        VariableConstraint(S->Context.bool_val(false)),
        MixedConstraint(MixedConstraint), S(S){};

  /// This method adds the constraint after visiting the constraint and
  /// constructing it while visiting.
  ///
  /// \param C The constraint to visit
  /// \param NegateExpr Whether the expression should be evaluated as is or
  /// negated
  /// \param RequireAll Whether all binary features have to be selected
  /// in order to evaluate the mixed constraint or not
  ///
  /// \returns \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool addConstraint(vara::feature::Constraint *C, bool NegateExpr = false,
                     bool RequireAll = true);

  /// Visits the binary constraint. Thereby, it constructs the z3 constraint
  /// by first executing the first part of the binary operator and afterwards
  /// the second (right) part.
  ///
  /// \param C the binary constraint to be converted
  ///
  /// \returns \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::BinaryConstraint *C) override;

  /// Visits the unary constraints. Thereby, it visits the operand and creates
  /// a new z3 constraint by preceeding it with the according expression.
  ///
  /// \param C the unary constraint to visit
  ///
  /// \returns \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::UnaryConstraint *C) override;

  /// Visits the feature in the constraint. This method creates the according
  /// z3 constant.
  ///
  /// \param C the primary feature
  ///
  /// \returns \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(vara::feature::PrimaryFeatureConstraint *C) override;

  /// Visits the feature in the integer constraints. This method creates the
  /// according z3 constant.
  ///
  /// \param C the integer constraint
  ///
  /// \returns \c true if adding the constraint was successfull; \c false
  /// otherwise.
  bool visit(feature::PrimaryIntegerConstraint *C) override;

private:
  /// The z3 constraint will be adjusted while visiting the given constraint
  z3::expr Z3ConstraintExpression;

  /// This expression variable is used to store an or expression including
  /// binary variables. This is needed for the 'req=all' attribute in
  /// mixed constraints.
  z3::expr VariableConstraint;

  /// This boolean is used to distinguish whether the visitor is used for
  /// a mixed constraint or not.
  bool MixedConstraint;

  /// The reference to the solver, which is basically needed for the context.
  /// Note that the context can not be copied or references, which is why we
  /// do not hold a direct reference to the context here.
  Z3Solver *S;
};

} // namespace vara::solver

#endif // VARA_SOLVER_Z3SOLVER_H_
