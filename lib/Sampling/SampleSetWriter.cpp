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
} // namespace vara::sampling
