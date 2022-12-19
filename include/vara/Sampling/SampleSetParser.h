#ifndef VARA_SAMPLING_SAMPLINGPARSER_H
#define VARA_SAMPLING_SAMPLINGPARSER_H

#include "vara/Configuration/Configuration.h"
#include "vara/Feature/FeatureModel.h"

#include "csv.hpp"

#include <memory>
#include <vector>

namespace vara::sampling {

class SampleSetParser {
public:
  /// This method parses the given csv file as a string. This csv file contains
  /// configurations from a sample set.
  /// \param Model the corresponding feature model
  /// \param csv the path to the csv file as a string
  /// \return a pointer to a vector containing the configurations from the
  /// sample set.
  [[nodiscard]] static std::unique_ptr<
      std::vector<std::unique_ptr<vara::feature::Configuration>>>
  readConfigurations(const feature::FeatureModel &Model, llvm::StringRef Csv);
};

} // namespace vara::sampling

#endif // VARA_SAMPLING_SAMPLINGPARSER_H
