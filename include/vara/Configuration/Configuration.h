#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include "vara/Configuration/ConfigurationOption.h"

namespace vara::feature {

class Configuration {
public:
  explicit Configuration() = default;
  virtual ~Configuration() = default;

  [[nodiscard]] static std::unique_ptr<Configuration> createConfigurationFromString(std::string ConfigurationString);

  void addConfigurationOption(std::unique_ptr<ConfigurationOption> Option);
  void setConfigurationOption(std::string Name, std::string Value);

  [[nodiscard]] std::string getConfigurationOptionValue(std::string Name);

  [[nodiscard]] std::vector<std::unique_ptr<ConfigurationOption>> getConfigurationOptions();

  [[nodiscard]] std::string dumpToString();

private:
  std::map<std::string, std::unique_ptr<ConfigurationOption>> OptionMapping;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
