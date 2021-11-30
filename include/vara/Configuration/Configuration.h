#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include "vara/Configuration/ConfigurationOption.h"

namespace vara::feature {

class Configuration {
 protected:
  explicit Configuration() = default;
 public:
  virtual ~Configuration() = default;

  [[nodiscard]] static Configuration createConfigurationFromString(std::string ConfigurationString);

  void addConfigurationOption(ConfigurationOption Option);
  void setConfigurationOption(std::string Name, std::string Value);

  [[nodiscard]] std::string getConfigurationOptionValue(std::string Name);

  [[nodiscard]] std::vector<ConfigurationOption> getConfigurationOptions();

  [[nodiscard]] std::string dumpToString();

 private:
  std::map<std::string, ConfigurationOption> OptionMapping;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
