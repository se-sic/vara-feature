#ifndef VARA_SOLVER_CONFIGURATIONFACTORY_H
#define VARA_SOLVER_CONFIGURATIONFACTORY_H

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Sampling/SamplingMethods.h"
#include "vara/Solver/Solver.h"
#include "vara/Solver/SolverFactory.h"

#include <vector>

namespace vara::solver {

/// This class represents a configuration iterator that lazily traverses
/// over all configurations.
class ConfigurationIterator {
public:
  class Iterator {
  public:
    Iterator(Solver *S = nullptr) : S{S} {}

    Result<SolverErrorCode, std::unique_ptr<vara::feature::Configuration>>
    operator*() const {
      return S->getCurrentConfiguration();
    }

    Iterator &operator++() {
      auto R = S->getNextConfiguration();
      if (!R) {
        S = nullptr;
      }
      return *this;
    }

    bool operator==(Iterator Other) const { return S == Other.S; }
    bool operator!=(Iterator Other) const { return !(Other == *this); }

  private:
    Solver *S;
  };

  explicit ConfigurationIterator(std::unique_ptr<Solver> Solver)
      : S(std::move(Solver)) {}

  virtual ~ConfigurationIterator() = default;

  Iterator begin() { return Iterator{S.get()}; }

  Iterator end() { return Iterator{}; } // NOLINT

private:
  std::unique_ptr<Solver> S;
};

/// This class is used to retrieve information about the feature model
/// (i.e., how many configurations are there? is it valid?)
class ConfigurationFactory {
public:
  /// This method returns an iterator. This iterator can be used to lazily
  /// traverse over configurations retrieved by the solver.
  ///
  /// \param Model the given model containing the features and constraints
  /// \param Type the type of solver to use
  ///
  /// \returns A unique pointer to the configuration iterator
  static ConfigurationIterator
  getConfigIterator(feature::FeatureModel &Model,
                    const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    return ConfigurationIterator(std::move(S));
  }

  /// This method returns all configurations of the given feature model. Note
  /// that retrieving all configurations might take some time or even be
  /// infeasible, especially for larger feature models.
  ///
  /// \param Model the given model containing the features and constraints
  /// \param Type the type of solver to use
  ///
  /// \returns a unique pointer to the vector containing all configurations
  static Result<SolverErrorCode,
                std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getAllConfigs(feature::FeatureModel &Model,
                const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    return S->getAllValidConfigurations();
  }

  /// This method returns not all but the specified amount of configurations.
  /// Note that the vector can also contain less configurations if there
  /// are not enough valid configurations.
  ///
  /// \param Model the given model containing the features and constraints
  /// \param N the number of configurations to retrieve
  /// \param Type the type of the solver
  ///
  /// \returns a unique pointer to a vector containing the specified amount of
  /// configurations.
  static Result<SolverErrorCode,
                std::vector<std::unique_ptr<vara::feature::Configuration>>>
  getNConfigs(feature::FeatureModel &Model, uint N,
              const vara::solver::SolverType Type = SolverType::Z3) {
    auto V = std::vector<std::unique_ptr<feature::Configuration>>();
    if (N == 0) {
      return V;
    }
    auto Iterator = ConfigurationFactory::getConfigIterator(Model, Type);
    for (auto R : Iterator) {
      if (R) {
        V.insert(V.begin(), R.extractValue());
      }
      if (V.size() == N) {
        break;
      }
    }
    return V;
  }

  /// This method returns whether the given model has valid configurations or
  /// not.
  ///
  /// \param Model the model containing the features and constraints
  /// \param Type the type of solver to use
  ///
  /// \returns true iff there is at least one valid configuration
  static bool isValid(feature::FeatureModel &Model,
                      const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    return S->hasValidConfigurations();
  }
};

} // namespace vara::solver

#endif // VARA_SOLVER_CONFIGURATIONFACTORY_H
