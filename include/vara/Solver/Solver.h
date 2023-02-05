#ifndef VARA_SOLVER_SOLVER_H_
#define VARA_SOLVER_SOLVER_H_

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

#include "vara/Feature/Relationship.h"

namespace vara::solver {

//===----------------------------------------------------------------------===//
//                               Solver Class
//===----------------------------------------------------------------------===//

/// \brief This class represents an interface for solvers.
class Solver {
public:
  explicit Solver() = default;

  virtual ~Solver() = default;

  /// Adds the given feature to the solver. That is, a new variable is created
  /// in the solver and all its constraints are included.
  /// The second argument indicates whether the 'optional' constraint should
  /// be considered or not -- note that this is needed since alternative
  /// features are modelled as not optional, but are neither optional nor
  /// mandatory.

  /// \returns a possible error if adding the feature was not successful.
  virtual Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd,
             bool IsInAlternativeGroup = false) = 0;

  /// This method adds a boolean feature with the given name. Please be aware
  /// that this method does only include a boolean feature with no constraints.
  /// If this is not intended, use the other methods, instead.

  /// \returns a possible error if adding the boolean feature was not
  /// successful.
  virtual Result<SolverErrorCode> addFeature(const string &FeatureName) = 0;

  /// Adds a numeric feature with the given name and the given values.
  /// Be aware that this method includes a numeric feature with no constraints
  /// and the specified values. That is also the case if the values of the
  /// feature are 0 and 1.
  /// If this is not intended, use the other methods, instead.

  /// \returns a possible error if adding the numeric feature was not
  /// successful.
  virtual Result<SolverErrorCode>
  addFeature(const string &FeatureName, const std::vector<int64_t> &Values) = 0;

  /// This method removes the given feature and all its constraints the feature
  /// is included in.

  /// \returns a possible error if removing the feature was not successful.
  virtual Result<SolverErrorCode>
  removeFeature(feature::Feature &FeatureToRemove) = 0;

  /// Processes the given relationship. This can be either an ALTERNATIVE group
  /// or an OR group.

  /// \param R the relationship to add.

  /// \returns a possible error if adding the relationship could not be done.
  virtual Result<SolverErrorCode>
  addRelationship(const feature::Relationship &R) = 0;

  /// Adds the given constraint to the solver. In the case that the constraint
  /// includes features that were not included yet, a error is returned.
  /// So, make sure that all features are already added.

  /// \returns a possible error if adding the constraint could not be done.
  virtual Result<SolverErrorCode>
  addConstraint(feature::Constraint &ConstraintToAdd) = 0;

  /// Adds the given mixedconstraint to the solver. In the case that the
  /// constraint includes features that were not included yet, a error is
  /// returned. So, make sure that all features are already added.
  /// Note that the difference to a normal constraint is that

  /// \returns a possible error if adding the constraint could not be done.
  virtual Result<SolverErrorCode>
  addMixedConstraint(feature::Constraint &ConstraintToAdd,
                     feature::FeatureModel::MixedConstraint::ExprKind ExprKind,
                     feature::FeatureModel::MixedConstraint::Req Req) = 0;

  /// Returns \c true if the current constraint system (i.e., its features and
  /// its constraints) has valid configurations.

  /// \returns an error if, for instance, not all constraints could be parsed
  /// yet because of missing variables. Otherwise, it contains a boolean whether
  /// the current constraint system is solvable (\c true) or not (\c false).
  virtual Result<SolverErrorCode, bool> hasValidConfigurations() = 0;

  /// Returns the number of valid configurations of the current constraint
  /// system (i.e., its features and its constraints). In principle, this is a
  /// #SAT call (i.e., enumerating all configurations).

  /// \returns an error if the number of valid configurations can not be
  /// retried. This can be the case if there are still constraints left that
  /// were not included into the solver because of missing variables.
  virtual Result<SolverErrorCode, std::unique_ptr<uint64_t>>
  getNumberValidConfigurations() = 0;

  /// Returns the current configuration.

  /// \returns the current configuration found by the solver an error code in
  /// case of error.
  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getCurrentConfiguration() = 0;

  /// This method returns the next configuration or an error in case of an
  /// error.

  /// \returns the next configuration or an error (e.g., if it is
  /// unsatisfiable).
  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() = 0;

  /// Returns all valid configurations. In comparison to \c
  /// getNumberValidConfigurations, this method returns the configurations
  /// instead of a number of configurations.

  /// \returns an error if an error occurs while retrieving the configurations.
  /// Otherwise, it will return the configurations.
  virtual Result<SolverErrorCode,
                 std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getAllValidConfigurations() = 0;
};

} // namespace vara::solver

#endif // VARA_SOLVER_SOLVER_H_
