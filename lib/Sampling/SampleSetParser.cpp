#include "vara/Sampling/SampleSetParser.h"

#include "stats.hpp"

namespace vara::sampling {

std::unique_ptr<std::vector<std::unique_ptr<vara::feature::Configuration>>>
SampleSetParser::readConfigurations(const feature::FeatureModel &Model,
                                    llvm::StringRef CsvPath) {
  csv::CSVFormat Format;
  Format.delimiter(';');
  auto V = std::make_unique<
      std::vector<std::unique_ptr<vara::feature::Configuration>>>();
  csv::CSVReader Reader(CsvPath, Format);
  for (const csv::CSVRow &Row : Reader) {
    auto Config = std::make_unique<vara::feature::Configuration>();
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
