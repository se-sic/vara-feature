#include "vara/Configuration/Configuration.h"

#include "llvm/Support/JSON.h"

#include <sstream>

namespace vara::feature {

std::shared_ptr<Configuration>
Configuration::createConfigurationFromString(std::string ConfigurationString) {
  std::shared_ptr<Configuration> configuration =
      std::make_unique<Configuration>();
  // Read in the string using the json library
  llvm::Expected<llvm::json::Value> parsedConfiguration =
      llvm::json::parse(llvm::StringRef(ConfigurationString));

  // If there was an error while parsing...
  if (!parsedConfiguration) {
    llvm::errs() << "The given configuration is not in the json format.\n";
    return nullptr;
  }
  llvm::json::Value value = parsedConfiguration.get();
  llvm::json::Value::Kind kind = value.kind();
  if (kind != llvm::json::Value::Kind::Object) {
    llvm::errs() << "The provided json has to be a map from the name to the "
                    "value (as string).\n";
    return nullptr;
  }
  llvm::json::Object *obj = value.getAsObject();
  for (auto iterator = obj->begin(); iterator != obj->end(); iterator++) {
    std::string first = iterator->getFirst().str();
    if (iterator->getSecond().kind() != llvm::json::Value::Kind::String) {
      llvm::errs() << "The values of the provided json string have to be "
                      "simple strings.\n";
      return nullptr;
    }
    std::string second = iterator->getSecond().getAsString()->str();
    std::shared_ptr<ConfigurationOption> option =
        std::make_unique<ConfigurationOption>(first, second);
    configuration->addConfigurationOption(std::move(option));
  }
  return configuration;
}

void Configuration::addConfigurationOption(
    std::shared_ptr<ConfigurationOption> Option) {
  this->OptionMapping[Option->getName()] = std::move(Option);
}

void Configuration::setConfigurationOption(std::string Name,
                                           std::string Value) {
  std::shared_ptr<ConfigurationOption> option =
      std::make_unique<ConfigurationOption>(Name, Value);
  addConfigurationOption(std::move(option));
}

std::string Configuration::getConfigurationOptionValue(std::string Name) {
  auto iterator = this->OptionMapping.find(Name);
  if (iterator == this->OptionMapping.end()) {
    llvm::errs() << "The configuration option " << Name << " does not exist.\n";
    return "";
  }
  return iterator->second->getValue();
}

std::vector<std::shared_ptr<ConfigurationOption>>
Configuration::getConfigurationOptions() {
  std::vector<std::shared_ptr<ConfigurationOption>> options{};
  for (auto &iterator : this->OptionMapping) {
    options.insert(options.begin(), iterator.second);
  }
  return options;
}

std::string Configuration::dumpToString() {
  llvm::json::Object obj{};
  for (auto &iterator : this->OptionMapping) {
    obj[iterator.first] = iterator.second->getValue();
  }
  llvm::json::Value value(std::move(obj));
  std::string dumped_string;
  llvm::raw_string_ostream rs(dumped_string);
  rs << value;
  rs.flush();
  return dumped_string;
}

} // namespace vara::feature