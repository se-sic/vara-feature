#include "vara/Configuration/Configuration.h"

#include "llvm/Support/JSON.h"

namespace vara::feature {

Configuration Configuration::createConfigurationFromString(std::string ConfigurationString) {
  Configuration configuration{};
  // Read in the string using the json library
  llvm::Expected<llvm::json::Value> parsed_configuration = llvm::json::parse(llvm::StringRef(ConfigurationString));

  // If there was an error while parsing...
  if (parsed_configuration) {
    llvm::errs() << "Failed to read in the given configuration.\n";
    return configuration;
  }
  llvm::json::Value value = parsed_configuration.get();
  return configuration;
}

void Configuration::addConfigurationOption(ConfigurationOption Option) {

}

void Configuration::setConfigurationOption(std::string Name, std::string Value) {

}

std::string Configuration::getConfigurationOptionValue(std::string Name) {
  return "";
}

std::vector<ConfigurationOption> Configuration::getConfigurationOptions() {
  return std::vector<ConfigurationOption>();
}

std::string Configuration::dumpToString() {
  return "";
}

}