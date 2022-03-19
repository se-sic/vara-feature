#ifndef VARA_FEATURE_CONFIGURATIONFACTORY_H
#define VARA_FEATURE_CONFIGURATIONFACTORY_H

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/FeatureModel.h"
#include "vara/Sampling/SamplingMethods.h"

#include <vector>

namespace vara::solver {

class ConfigurationFactory {
public:
  // TODO: Iterator zurückgeben um lazy über die Konfiguration zu gehen
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
