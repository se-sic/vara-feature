#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include <llvm/Support/FormatVariadic.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <variant>

namespace vara::feature {

/// \brief This class represents a simple configuration option.
///
/// Note that this class is not linked to real features from the feature model
/// and does also not check the type and the value of the given option.
class ConfigurationOption {
public:
  virtual ~ConfigurationOption() = default;

  ConfigurationOption(llvm::StringRef Name, llvm::StringRef Value)
      : Name(Name), Value(convert(Value)){};

  /// This method returns the name of the configuration option.
  /// \returns the name of the configuration option as a string
  [[nodiscard]] llvm::StringRef getName() { return this->Name; }

  /// This method returns the string value of the configuration option.
  /// \returns the string value of the configuration option as a string
  [[nodiscard]] llvm::StringRef getStringValue() {
    if (isString()) {
      return std::get<llvm::StringRef>(this->Value);
    }
    llvm::errs() << "The value is not a string!\n";
    return "";
  }

  /// This method returns the boolean value of the configuration option.
  /// \returns the boolean value of the configuration option as a bool
  [[nodiscard]] bool getBoolValue() {
    if (isBool()) {
      return std::get<bool>(this->Value);
    }
    llvm::errs() << "The value is not a boolean!\n";
    return false;
  }

  /// This method returns the integer value of the configuration option.
  /// \returns the integer value of the configuration option as an int64_t
  [[nodiscard]] int64_t getIntValue() {
    if (isInt()) {
      return std::get<int64_t>(this->Value);
    }
    llvm::errs() << "The value is not an integer!\n";
    return -1;
  }

  /// This method returns the value as string no matter what the real type is.
  /// \returns the value as string
  [[nodiscard]] std::string getValueAsString() {
    std::string ConvertedValue;
    if (isString()) {
      ConvertedValue = getStringValue();
    } else if (isBool()) {
      ConvertedValue = std::get<bool>(this->Value) ? "true" : "false";
    } else {
      std::ostringstream O;
      O << std::get<int64_t>(this->Value);
      ConvertedValue = O.str();
    }
    return ConvertedValue;
  }

  /// This method returns the string representation of this configuration
  /// option.
  /// \returns the string representation in the format: <code>name:
  /// value</code>
  [[nodiscard]] std::string toString() {
    return llvm::formatv("{0}: {1}", this->Name, getValueAsString());
  }

  /// This method returns \c `true` if the value is a boolean value
  /// \returns whether the value of the configuration option is a boolean value
  [[nodiscard]] bool isBool() { return std::holds_alternative<bool>(Value); }

  /// This method returns \c `true` if the value is a string
  /// \returns whether the value of the configuration option is a string
  [[nodiscard]] bool isString() {
    return std::holds_alternative<llvm::StringRef>(Value);
  }

  /// This method returns \c `true` if the value is an integer
  /// \returns whether the value of the configuration option is an integer
  [[nodiscard]] bool isInt() { return std::holds_alternative<int64_t>(Value); }

private:
  /// The name of the configuration option
  const llvm::StringRef Name;

  /// The value of the configuration option
  std::variant<bool, int64_t, llvm::StringRef> Value;

  /// This method parses the given string and tries to convert it.
  /// \returns a variant of the most specific type (int64_t, bool, or StringRef)
  [[nodiscard]] static std::variant<bool, int64_t, llvm::StringRef>
  convert(llvm::StringRef ValueToConvert) {
    // Parse the value
    if (ValueToConvert.lower() == "true" || ValueToConvert.lower() == "false") {
      return ValueToConvert == "true";
    }
    int64_t IntegerValue;
    if (!ValueToConvert.getAsInteger(0, IntegerValue)) {
      return IntegerValue;
    }
    return ValueToConvert;
  }
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
  [[nodiscard]] std::string
  getConfigurationOptionValue(const std::string &Name);

  /// This method dumps the current configuration to a json string.
  /// \returns the current configuration as a json-formatted string
  [[nodiscard]] std::string dumpToString();

private:
  /// This represents a mapping from the name of the option to the option
  /// object.
  std::map<llvm::StringRef, std::unique_ptr<ConfigurationOption>>
      OptionMappings;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
