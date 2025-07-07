#include "vara/Sampling/SampleSetWriter.h"

#include <llvm/ObjectYAML/YAML.h>
#include <llvm/Support/YAMLTraits.h>

LLVM_YAML_IS_STRING_MAP(std::string)

namespace vara::sampling {

std::string vara::sampling::SampleSetWriter::writeConfigurations(
    const vara::feature::FeatureModel &FM,
    std::vector<std::unique_ptr<vara::feature::Configuration>>
        &Configurations) {

  // Convert configurations into a map of string to string
  std::map<std::string, std::string> ConfigurationStringMap;
  for (size_t ConfigurationCount = 0;
       ConfigurationCount < Configurations.size(); ConfigurationCount++) {
    std::string ConfigurationStringFlags = "[";
    auto &Configuration = Configurations.at(ConfigurationCount);
    for (auto *F : FM.features()) {
      if (auto Value = Configuration->configurationOptionValue(F->getName());
          Value && Value.value() != "false" && !F->getOutputString().empty()) {
        if (ConfigurationStringFlags.size() != 1) {
          ConfigurationStringFlags.append(", ");
        }
        ConfigurationStringFlags.append("\"");
        ConfigurationStringFlags.append(F->getOutputString().str());
        if (F->getKind() == feature::Feature::FeatureKind::FK_NUMERIC) {
          auto OptionValue =
              Configuration->configurationOptionValue(F->getName());
          assert(
              OptionValue.has_value() &&
              "Could not retrieve option value, broken configuration option.");
          ConfigurationStringFlags.append(OptionValue.value());
        }
        ConfigurationStringFlags.append("\"");
      }
    }
    ConfigurationStringFlags.append("]");
    ConfigurationStringMap[std::to_string(ConfigurationCount)] =
        ConfigurationStringFlags;
  }

  // Write configurations to a string in YAML format
  std::string Str;
  llvm::raw_string_ostream OutputString(Str);
  llvm::yaml::Output Output(OutputString);
  Output << ConfigurationStringMap;
  return Str;
}

std::string vara::sampling::SampleSetWriterCSV::writeConfigurations(
    const vara::feature::FeatureModel &FM,
    std::vector<std::unique_ptr<vara::feature::Configuration>>
        &Configurations) {

  std::string HeaderStr = "id";

  for(auto *F : FM.features()) {
      HeaderStr.append(",");
      HeaderStr.append(F->getName());
  }

  HeaderStr.append("\n");

  std::vector<std::string> ConfigRows;
  ConfigRows.reserve(Configurations.size());

  for (size_t ConfigurationCount = 0;
       ConfigurationCount < Configurations.size(); ConfigurationCount++) {

    auto &Configuration = Configurations.at(ConfigurationCount);
    std::string ConfigRow = std::to_string(ConfigurationCount);

    for (auto *F : FM.features()) {
      auto FeatureName = F->getName();

      ConfigRow.append(",");

      auto Value = Configuration->configurationOptionValue(F->getName());

      assert(Value.has_value() && "Could not retrieve option value, broken configuration option.");

      ConfigRow.append(Value.value());
    }
    ConfigRow.append("\n");
    ConfigRows.push_back(ConfigRow);
  }

  // Write configurations to a string in YAML format
  std::string Str;
  llvm::raw_string_ostream OutputString(Str);

  OutputString << HeaderStr;

  for(auto &Config: ConfigRows){
    OutputString << Config;
  }

  return Str;
}
} // namespace vara::sampling
