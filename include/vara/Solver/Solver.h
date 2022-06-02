#ifndef VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_
#define VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

#include "z3++.h"

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
  /// \return a possible error if adding the feature was not successful.
  virtual Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd) = 0;

  /// This method adds a boolean feature with the given name. Please be aware
  /// that this method does only include a boolean feature with no constraints.
  /// If this is not intended, use the other methods, instead.
  /// \return a possible error if adding the boolean feature was not successful.
  virtual Result<SolverErrorCode> addFeature(string FeatureName) = 0;

  /// Adds a numeric feature with the given name and the given values.
  /// Be aware that this method includes a numeric feature with no constraints
  /// and the specified values. That is also the case if the values of the
  /// feature are 0 and 1.
  /// If this is not intended, use the other methods, instead.
  /// \return a possible error if adding the numeric feature was not successful.
  virtual Result<SolverErrorCode>
  addFeature(const string &FeatureName, const std::vector<int64_t> &Values) = 0;

  /// This method removes the given feature and all its constraints the feature
  /// is included in.
  /// \return a possible error if removing the feature was not successful.
  virtual Result<SolverErrorCode>
  removeFeature(const feature::Feature &FeatureToRemove) = 0;

  /// Adds the given constraint to the solver. In the case that the constraint
  /// includes features that were not included yet, a error is returned.
  /// So, make sure that all features are already added.
  /// \return a possible error if adding the constraint could not be done.
  virtual Result<SolverErrorCode>
  addConstraint(const feature::Constraint &ConstraintToAdd) = 0;

  /// Removes the given constraint from the solver.
  /// If the constraint is not found, an error is returned.
  /// \return a possible error if removing the constraint could not be done.
  virtual Result<SolverErrorCode>
  removeConstraint(const feature::Constraint &ConstraintToRemove) = 0;

  /// This method gets a partial configuration and searches for another
  /// valid configuration which minimizes the number of selected features.
  /// This is usually used for t-wise.
  /// \return a possible error if minimizing the configuration could not be
  /// done.
  virtual Result<SolverErrorCode>
  minimizeConfiguration(const feature::Configuration &Config) = 0;

  /// This method searches a maximum configuration for the given partial
  /// configuration.
  /// This is usually used for negative feature wise sampling.
  /// \return a possible error if maximizing the configuration could not be
  /// done.
  virtual Result<SolverErrorCode>
  maximizeConfiguration(const feature::Configuration &Config) = 0;

  // TODO: This could be redundant when replacing by a constraint (sum)
  // virtual bool addExactFeatureNumber(int NumberFeaturesEnabled) = 0;

  // TODO: Remove minimize/maximize/exactFeature constraint?

  /// Returns \c true if the current constraint system (i.e., its features and
  /// its constraints) has valid configurations.
  /// \return an error if, for instance, not all constraints could be parsed yet
  /// because of missing variables. Otherwise, it contains a boolean whether the
  /// current constraint system is solvable (\c true) or not (\c false).
  virtual Result<SolverErrorCode, bool *> hasValidConfigurations() = 0;

  /// Returns the number of valid configurations of the current constraint
  /// system (i.e., its features and its constraints). In principle, this is a
  /// #SAT call (i.e., enumerating all configurations).
  /// \return an error if the number of valid configurations can not be retried.
  /// This can be the case if there are still constraints left that were not
  /// included into the solver because of missing variables.
  virtual Result<SolverErrorCode, u_int64_t *> getNumberValidConfigurations() = 0;

  /// This method returns the next configuration or an error in case of an
  /// error. \return the next configuration or an error (e.g., if it is
  /// unsatisfiable).
  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() = 0;

  /// This method resets the configuration iterator.
  /// \return an error if resetting the configuration iterator should not be
  /// successful.
  virtual Result<SolverErrorCode> resetConfigurationIterator() = 0;

  /// Returns all valid configurations. In comparison to \c
  /// getNumberValidConfigurations, this method returns the configurations
  /// instead of a number of configurations.
  /// \return an error if an error occurs while retrieving the configurations.
  /// Otherwise, it will return the configurations.
  virtual Result<SolverErrorCode,
                 std::vector<std::unique_ptr<vara::feature::Configuration>> *>
  getAllValidConfigurations() = 0;
};

//===----------------------------------------------------------------------===//
//                               Z3Solver Class
//===----------------------------------------------------------------------===//

/// \brief The Z3 solver implementation for handling constraints
class Z3Solver : public Solver {
public:
  explicit Z3Solver() : Solver(Context) {}

  Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd) override;

  Result<SolverErrorCode> addFeature(string FeatureName) override;

  Result<SolverErrorCode>
  addFeature(const string &FeatureName,
             const std::vector<int64_t> &Values) override;

  Result<SolverErrorCode>
  removeFeature(const feature::Feature &FeatureToRemove) override;

  Result<SolverErrorCode>
  addConstraint(const feature::Constraint &ConstraintToAdd) override;

  Result<SolverErrorCode>
  removeConstraint(const feature::Constraint &ConstraintToRemove) override;

  Result<SolverErrorCode>
  minimizeConfiguration(const feature::Configuration &Config) override;

  Result<SolverErrorCode>
  maximizeConfiguration(const feature::Configuration &Config) override;

  Result<SolverErrorCode, bool *> hasValidConfigurations() override;

  Result<SolverErrorCode, u_int64_t *> getNumberValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() override;

  Result<SolverErrorCode> resetConfigurationIterator() override;

  Result<SolverErrorCode,
         std::vector<std::unique_ptr<vara::feature::Configuration>> *>
  getAllValidConfigurations() override;

private:
  /// This map contains the original feature names as key and maps it to the
  /// Z3 value.
  llvm::StringMap<std::unique_ptr<z3::expr>> OptionToVariableMapping;

  /// This vector contains the constraints that remain unprocessed because of
  /// features/variables that are not included yet into the solver.
  llvm::SmallVector<std::unique_ptr<feature::Constraint>>
      UnprocessedConstraints;

  /// The context of Z3 needed for every allocation.
  z3::context Context;

  /// The instance of the Z3 solver needed for caching the constraints and
  /// variables.
  z3::solver Solver;
};

} // namespace vara::solver

#endif // VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_
