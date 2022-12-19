#include "vara/Sampling/SampleSetParser.h"

#include "stats.hpp"

namespace vara::sampling {

std::unique_ptr<std::vector<std::unique_ptr<vara::feature::Configuration>>>
SampleSetParser::readConfigurations(const feature::FeatureModel &Model,
                                    llvm::StringRef Csv) {
  csv::CSVFormat Format;
  Format.delimiter(';');
  std::unique_ptr<std::vector<std::unique_ptr<vara::feature::Configuration>>> V;
  csv::CSVReader Reader(Csv, Format);
  for (csv::CSVRow &Row : Reader) {
    std::unique_ptr<vara::feature::Configuration> Config;
    auto ColumnNames = Row.get_col_names();
    for (auto *F : Model.features()) {
      if (std::find(ColumnNames.begin(), ColumnNames.end(), F->getName()) !=
          ColumnNames.end()) {
        Config->setConfigurationOption(
            F->getName(), llvm::StringRef(Row[F->getName().str()]));
      }
    }
    V->insert(V->begin(), std::move(Config));
  }
  return V;
}
} // namespace vara::sampling
