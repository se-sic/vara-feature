#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include "vara/Configuration/ConfigurationOption.h"

namespace vara::feature {

/// \brief This class represents a configuration.
/// The configuration is represented by using a json format of the form
/// \c `{name: value, name2: value2}`.
class Configuration {
public:
  explicit Configuration() = default;
  virtual ~Configuration() = default;

  /// This method creates a configuration from the provided json string.
  [[nodiscard]] static std::unique_ptr<Configuration> createConfigurationFromString(std::string ConfigurationString);

  /// This method adds a configuration option to the current configuration.
  void addConfigurationOption(std::unique_ptr<ConfigurationOption> Option);

  /// This method sets a configuration option by using the provided name and
  /// value.
  void setConfigurationOption(std::string Name, std::string Value);

  /// This method returns the value of the configuration option.
  /// \returns the value of the configuration option as a string
  [[nodiscard]] std::string getConfigurationOptionValue(std::string Name);

  /// This method returns all configuration options.
  /// \returns all configuration options in a vector
  [[nodiscard]] std::vector<std::unique_ptr<ConfigurationOption>> getConfigurationOptions();

  /// This method dumps the current configuration to a json string.
  /// \returns the current configuration as a json-formatted string
  [[nodiscard]] std::string dumpToString();

private:
  /// This represents a mapping from the name of the option to the option object.
  std::map<std::string, std::unique_ptr<ConfigurationOption>> OptionMapping;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
