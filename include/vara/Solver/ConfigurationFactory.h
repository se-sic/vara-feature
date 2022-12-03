#ifndef VARA_FEATURE_CONFIGURATIONFACTORY_H
#define VARA_FEATURE_CONFIGURATIONFACTORY_H

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

    bool operator==(Iterator other) const { return S == other.S; }
    bool operator!=(Iterator other) const { return !(other == *this); }

  private:
    Solver *S;
  };

  explicit ConfigurationIterator(std::unique_ptr<Solver> Solver)
      : Solver(std::move(Solver)) {}

  virtual ~ConfigurationIterator() = default;

  Iterator begin() { return Iterator{Solver.get()}; }

  Iterator end() { return Iterator{}; }

private:
  std::unique_ptr<Solver> Solver;
};

class ConfigurationFactory {
public:
  static std::unique_ptr<ConfigurationIterator>
  getConfigIterator(feature::FeatureModel &Model,
                    const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    std::unique_ptr<ConfigurationIterator> Iterator(
        new ConfigurationIterator(std::move(S)));
    return Iterator;
  }

  static Result<SolverErrorCode,
                std::unique_ptr<
                    std::vector<std::unique_ptr<vara::feature::Configuration>>>>
  getAllConfigs(feature::FeatureModel &Model,
                const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    return S->getAllValidConfigurations();
  }

  static Result<SolverErrorCode,
                std::unique_ptr<
                    std::vector<std::unique_ptr<vara::feature::Configuration>>>>
  getNConfigs(feature::FeatureModel &Model,
              sampling::SamplingMethod &SamplingMethod, uint N,
              const vara::solver::SolverType Type = SolverType::Z3) {
    auto Iterator = getConfigIterator(Model, Type);
    std::unique_ptr<std::vector<std::unique_ptr<feature::Configuration>>> V;
    if (N == 0) {
      return V;
    }
    for (auto R : (*Iterator)) {
      if (R) {
        V->insert(V->begin(), R.extractValue());
      }
      if (V->size() == N) {
        break;
      }
    }
  }

  static bool isValid(feature::FeatureModel &Model,
                      const vara::solver::SolverType Type = SolverType::Z3) {
    auto S = SolverFactory::initializeSolver(Model, Type);
    return S->hasValidConfigurations();
  }
};

} // namespace vara::solver

#endif // VARA_FEATURE_CONFIGURATIONFACTORY_H
