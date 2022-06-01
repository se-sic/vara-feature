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

  virtual Result<SolverErrorCode>
  addFeature(const feature::Feature &FeatureToAdd) = 0;

  virtual Result<SolverErrorCode> addFeature(string FeatureName) = 0;

  virtual Result<SolverErrorCode>
  addFeature(const string &FeatureName, const std::vector<int64_t> &Values) = 0;

  virtual Result<SolverErrorCode>
  removeFeature(const feature::Feature &FeatureToRemove) = 0;

  virtual Result<SolverErrorCode>
  addConstraint(const feature::Constraint &ConstraintToAdd) = 0;

  virtual Result<SolverErrorCode>
  removeConstraint(const feature::Constraint &ConstraintToRemove) = 0;

  // TODO: Document the use of partial configurations in relation to
  // Configuration class
  virtual Result<SolverErrorCode>
  minimizeConfiguration(const feature::Configuration &Config) = 0;

  virtual Result<SolverErrorCode>
  maximizeConfiguration(const feature::Configuration &Config) = 0;

  // TODO: This could be redundant when replacing by a constraint (sum)
  // virtual bool addExactFeatureNumber(int NumberFeaturesEnabled) = 0;

  // TODO: Remove minimize/maximize/exactFeature constraint?

  virtual bool hasValidConfigurations() = 0;

  virtual Result<SolverErrorCode, u_int64_t> getNumberValidConfigurations() = 0;

  virtual Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() = 0;

  virtual Result<SolverErrorCode,
                 std::vector<std::unique_ptr<vara::feature::Configuration>>>
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

  bool hasValidConfigurations() override;

  Result<SolverErrorCode, u_int64_t> getNumberValidConfigurations() override;

  Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() override;

  Result<SolverErrorCode,
         std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getAllValidConfigurations() override;

private:
  /// The context of Z3 needed for every allocation.
  z3::context Context;

  /// The instance of the Z3 solver needed for caching the constraints and
  /// variables.
  z3::solver Solver;
};

} // namespace vara::solver

#endif // VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_
