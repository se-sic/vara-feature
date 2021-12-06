#ifndef VARA_CONFIGURATION_CONFIGURATION_H
#define VARA_CONFIGURATION_CONFIGURATION_H

#include <llvm/ADT/StringMap.h>
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

  /// The constructor of a configuration options takes the name and the value
  /// as an argument. The name and the value do not change over time.
  ConfigurationOption(llvm::StringRef Name, llvm::StringRef Value)
      : Name(Name.str()), Value(convert(Value)){};

  /// This method returns the name of the configuration option.
  /// \returns the name of the configuration option as a string
  [[nodiscard]] llvm::StringRef name() const { return this->Name; }

  /// This method returns the string value of the configuration option.
  /// \returns the string value of the configuration option as a string
  [[nodiscard]] llvm::Optional<llvm::StringRef> stringValue() const {
    if (isString()) {
      return llvm::Optional<llvm::StringRef>{
          std::get<std::string>(this->Value)};
    }
    return llvm::Optional<llvm::StringRef>{};
  }

  /// This method returns the boolean value of the configuration option.
  /// \returns the boolean value of the configuration option as a bool
  [[nodiscard]] llvm::Optional<bool> boolValue() const {
    if (isBool()) {
      return llvm::Optional<bool>{std::get<bool>(this->Value)};
    }
    return llvm::Optional<bool>{};
  }

  /// This method returns the integer value of the configuration option.
  /// \returns the integer value of the configuration option as an int64_t
  [[nodiscard]] llvm::Optional<int64_t> intValue() const {
    if (isInt()) {
      return llvm::Optional<int64_t>{std::get<int64_t>(this->Value)};
    }
    return llvm::Optional<int64_t>{};
  }

  /// This method returns the value as string no matter what the real type is.
  /// \returns the value as string
  [[nodiscard]] std::string asString() const {
    std::string ConvertedValue;
    if (isString()) {
      ConvertedValue = stringValue().getValue();
    } else if (isBool()) {
      if (boolValue().getValue()) {
        ConvertedValue = "true";
      } else {
        ConvertedValue = "false";
      }
    } else {
      ConvertedValue = std::to_string(intValue().getValue());
    }
    return ConvertedValue;
  }

  /// This method returns the string representation of this configuration
  /// option.
  /// \returns the string representation in the format: <code>name:
  /// value</code>
  [[nodiscard]] std::string toString() const {
    return llvm::formatv("{0}: {1}", this->Name, asString());
  }

  /// This method returns \c `true` if the value is a boolean value
  /// \returns whether the value of the configuration option is a boolean value
  [[nodiscard]] bool isBool() const {
    return std::holds_alternative<bool>(Value);
  }

  /// This method returns \c `true` if the value is a string
  /// \returns whether the value of the configuration option is a string
  [[nodiscard]] bool isString() const {
    return std::holds_alternative<std::string>(Value);
  }

  /// This method returns \c `true` if the value is an integer
  /// \returns whether the value of the configuration option is an integer
  [[nodiscard]] bool isInt() const {
    return std::holds_alternative<int64_t>(Value);
  }

private:
  /// This method parses the given string and tries to convert it.
  /// \returns a variant of the most specific type (int64_t, bool, or StringRef)
  [[nodiscard]] static std::variant<bool, int64_t, std::string>
  convert(llvm::StringRef ValueToConvert) {
    // Parse the value
    if (ValueToConvert.lower() == "true" || ValueToConvert.lower() == "false") {
      return ValueToConvert == "true";
    }
    int64_t IntegerValue;
    if (!ValueToConvert.getAsInteger(0, IntegerValue)) {
      return IntegerValue;
    }
    return ValueToConvert.str();
  }

  /// The name of the configuration option
  const std::string Name;

  /// The value of the configuration option
  const std::variant<bool, int64_t, std::string> Value;
};

/// \brief This class represents a configuration.
///
/// The configuration is represented by using a json format of the form
/// \c `{name: value, name2: value2}`.
class Configuration {
public:
  Configuration() = default;
  virtual ~Configuration() = default;

  [[nodiscard]] llvm::StringMapIterator<std::unique_ptr<ConfigurationOption>>
  begin() {
    return OptionMappings.begin();
  }
  [[nodiscard]] llvm::StringMapConstIterator<
      std::unique_ptr<ConfigurationOption>>
  begin() const {
    return OptionMappings.end();
  }

  [[nodiscard]] llvm::StringMapIterator<std::unique_ptr<ConfigurationOption>>
  end() {
    return OptionMappings.end();
  }
  [[nodiscard]] llvm::StringMapConstIterator<
      std::unique_ptr<ConfigurationOption>>
  end() const {
    return OptionMappings.end();
  }

  /// This method creates a configuration from the provided json string.
  [[nodiscard]] static std::unique_ptr<Configuration>
  createConfigurationFromString(llvm::StringRef ConfigurationString);

  /// This method adds a configuration option to the current configuration.
  void addConfigurationOption(std::unique_ptr<ConfigurationOption> Option);

  /// This method sets a configuration option by using the provided name and
  /// value.
  void setConfigurationOption(llvm::StringRef Name, llvm::StringRef Value);

  /// This method returns the value of the configuration option.
  /// \returns the value of the configuration option as a string
  [[nodiscard]] llvm::Optional<std::string>
  configurationOptionValue(llvm::StringRef Name);

  /// This method dumps the current configuration to a json string.
  /// \returns the current configuration as a json-formatted string
  [[nodiscard]] std::string dumpToString();

private:
  /// This represents a mapping from the name of the option to the option
  /// object.
  llvm::StringMap<std::unique_ptr<ConfigurationOption>> OptionMappings;
};

} // namespace vara::feature

#endif // VARA_CONFIGURATION_CONFIGURATION_H
