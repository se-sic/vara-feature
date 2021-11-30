#ifndef VARA_CONFIGURATION_CONFIGURATIONOPTION_H
#define VARA_CONFIGURATION_CONFIGURATIONOPTION_H

#include <llvm/Support/FormatVariadic.h>

#include <map>
#include <string>
#include <vector>

using std::string;

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
  [[nodiscard]] std::string getName() { return this->Name; }

  /// This method returns the value of the configuration option.
  /// \returns the value of the configuration option as a string
  [[nodiscard]] std::string getValue() { return this->Value; }

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
} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATIONOPTION_H
