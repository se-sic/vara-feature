#ifndef VARA_SAMPLING_SAMPLESETWRITER_H
#define VARA_SAMPLING_SAMPLESETWRITER_H

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/FeatureModel.h"

#include <memory>
#include <vector>

namespace vara::sampling {

class SampleSetWriter {
public:
  /// This method writes the given configurations to a yml file.
  ///
  /// \param FM the corresponding feature model
  /// \param Configurations the configurations to print into the file
  ///
  /// \return the string containing the output in YAML format
  [[nodiscard]] static std::string
  writeConfigurations(const vara::feature::FeatureModel &FM,
                      std::vector<std::unique_ptr<vara::feature::Configuration>>
                          &Configurations);
};

} // namespace vara::sampling

#endif // VARA_SAMPLING_SAMPLESETWRITER_H
