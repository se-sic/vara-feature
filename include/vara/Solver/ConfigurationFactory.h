#ifndef VARA_FEATURE_CONFIGURATIONFACTORY_H
#define VARA_FEATURE_CONFIGURATIONFACTORY_H

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Sampling/SamplingMethods.h"
#include "vara/Solver/Solver.h"

#include <vector>

namespace vara::solver {

class ConfigurationIterator {
public:
  explicit ConfigurationIterator(Solver &Solver) : Solver(Solver) {}

  virtual ~ConfigurationIterator() = default;

  ConfigurationIterator &operator++() {
    Solver.getNextConfiguration();
    // TODO: Set constraint for next configuration
    return *this;
  }

  ConfigurationIterator operator++(int) {
    ConfigurationIterator retval = *this;
    ++(*this);
    return retval;
  }

  bool operator==(ConfigurationIterator other) const {
    return &Solver == &other.Solver;
  }

  bool operator!=(ConfigurationIterator other) const {
    return !(*this == other);
  }

  Solver &operator*() { return Solver; }

private:
  Solver &Solver;
};

class ConfigurationFactory {
public:
  // TODO: Iterator zurückgeben um lazy über die Konfiguration zu gehen
  static std::unique_ptr<ConfigurationIterator> getConfigIterator();

  static std::vector<feature::Configuration>
  getAllConfigs(feature::FeatureModel &Model);

  static std::vector<feature::Configuration>
  getNConfigs(feature::FeatureModel &Model,
              sampling::SamplingMethod &SamplingMethod, int N);

  static bool isValid(feature::FeatureModel &Model);

  // TODO: ValidBuilder
};

} // namespace vara::solver

#endif // VARA_FEATURE_CONFIGURATIONFACTORY_H
