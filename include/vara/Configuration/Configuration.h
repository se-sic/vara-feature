#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include <llvm/Support/FormatVariadic.h>
#include <map>
#include <memory>
#include <string>

namespace vara::feature {

/// \brief This class represents a simple configuration option.
///
/// Note that this class is not linked to real features from the feature model
/// and does also not check the type and the value of the given option.
class ConfigurationOption {
public:
  virtual ~ConfigurationOption() = default;

  ConfigurationOption(std::string Name, std::string Value)
      : Name(std::move(Name)), Value(std::move(Value)){};

  /// This method returns the name of the configuration option.
  /// \returns the name of the configuration option as a string
  [[nodiscard]] std::string_view getName() { return this->Name; }

  /// This method returns the value of the configuration option.
  /// \returns the value of the configuration option as a string
  [[nodiscard]] std::string_view getValue() { return this->Value; }

  /// This method returns the string representation of this configuration
  /// option. \returns the string representation in the format: <code>name:
  /// value</code>
  [[nodiscard]] std::string toString() {
    return llvm::formatv("{0}: {1}", this->Name, this->Value);
  }

private:
  /// The name of the configuration option
  const std::string Name;

  /// The value of the configuration option
  const std::string Value;
};

/// \brief This class represents a configuration.
///
/// The configuration is represented by using a json format of the form
/// \c `{name: value, name2: value2}`.
class Configuration {
public:
  Configuration() = default;
  virtual ~Configuration() = default;

  /// This method creates a configuration from the provided json string.
  [[nodiscard]] static std::unique_ptr<Configuration>
  createConfigurationFromString(llvm::StringRef ConfigurationString);

  /// This method adds a configuration option to the current configuration.
  void addConfigurationOption(std::unique_ptr<ConfigurationOption> Option);

  /// This method sets a configuration option by using the provided name and
  /// value.
  void setConfigurationOption(const std::string &Name,
                              const std::string &Value);

  /// This method returns the value of the configuration option.
  /// \returns the value of the configuration option as a string
  [[nodiscard]] std::string_view
  getConfigurationOptionValue(const std::string &Name);

  /// This method dumps the current configuration to a json string.
  /// \returns the current configuration as a json-formatted string
  [[nodiscard]] std::string dumpToString();

private:
  /// This represents a mapping from the name of the option to the option
  /// object.
  std::map<std::string_view, std::unique_ptr<ConfigurationOption>>
      OptionMappings;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
