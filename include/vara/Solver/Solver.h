#ifndef VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_
#define VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/Constraint.h"
#include "vara/Feature/Feature.h"
#include "vara/Solver/Error.h"
#include "vara/Utils/Result.h"

namespace vara::solver {

/// \brief This class represents an interface for solvers.
class Solver {
public:
  explicit Solver() = default;

  virtual ~Solver() = default;
  // TODO: Errorhandlingmonade Error und ErrorResult
  // const feature ref überall
  virtual bool addFeature(const feature::Feature &FeatureToAdd) = 0;

  // TODO: Möglichkeit zum Hinzufügen von Featuren mit Rohdaten (Name, Werte)
  // TODO: Overload für Name als Argument
  virtual Result<SolverErrorCode> addFeature(const string FeatureName) = 0;

  virtual Result<SolverErrorCode>
  addFeature(const string FeatureName, const std::vector<int64_t> Values) = 0;

  virtual Result<SolverErrorCode>
  removeFeature(feature::Feature FeatureToRemove) = 0;

  virtual Result<SolverErrorCode>
  addConstraint(feature::Constraint ConstraintToAdd) = 0;

  virtual Result<SolverErrorCode>
  removeConstraint(feature::Constraint ConstraintToRemove) = 0;

  // TODO: Document the use of partial configurations in relation to
  // Configuration class
  virtual Result<SolverErrorCode>
  minimizeConfiguration(feature::Configuration Config) = 0;

  virtual Result<SolverErrorCode>
  maximizeConfiguration(feature::Configuration Config) = 0;

  // TODO: This could be redundant when replacing by a constraint (sum)
  // virtual bool addExactFeatureNumber(int NumberFeaturesEnabled) = 0;

  // TODO: Remove minimize/maximize/exactFeature constraint?

  virtual bool hasValidConfigurations() = 0;

  // TODO: #SAT mit Optional<unsigned> or Error -- UNSAT
  // virtual Optional getNumber
  virtual llvm::Optional<u_int64_t> getNumberValidConfigurations() = 0;

  // TODO: Return-Wert falsch
  // TODO: solve für eine Konfiguration und solve für alle
  virtual llvm::Optional<std::unique_ptr<vara::feature::Configuration>>
  getNextConfiguration() = 0;

  virtual llvm::Optional<
      std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getAllValidConfigurations() = 0;
};

} // namespace vara::solver

#endif // VARA_FEATURE_INCLUDE_VARA_SOLVER_SOLVER_H_
