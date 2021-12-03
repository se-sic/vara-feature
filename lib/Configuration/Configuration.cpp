#include "vara/Configuration/Configuration.h"

#include "llvm/Support/JSON.h"

#include <sstream>

namespace vara::feature {

std::shared_ptr<Configuration> Configuration::createConfigurationFromString(
    llvm::StringRef ConfigurationString) {
  std::shared_ptr<Configuration> Conf = std::make_unique<Configuration>();
  // Read in the string using the json library
  llvm::Expected<llvm::json::Value> ParsedConfiguration =
      llvm::json::parse(ConfigurationString);

  // If there was an error while parsing...
  if (!ParsedConfiguration) {
    llvm::errs() << "The given configuration is not in the json format.\n";
    return nullptr;
  }
  llvm::json::Value Value = ParsedConfiguration.get();
  llvm::json::Value::Kind Kind = Value.kind();
  if (Kind != llvm::json::Value::Kind::Object) {
    llvm::errs() << "The provided json has to be a map from the name to the "
                    "value (as string).\n";
    return nullptr;
  }
  llvm::json::Object *Obj = Value.getAsObject();
  for (auto Iterator = Obj->begin(); Iterator != Obj->end(); Iterator++) {
    std::string First = Iterator->getFirst().str();
    if (Iterator->getSecond().kind() != llvm::json::Value::Kind::String) {
      llvm::errs() << "The values of the provided json string have to be "
                      "simple strings.\n";
      return nullptr;
    }
    std::string Second = Iterator->getSecond().getAsString()->str();
    std::shared_ptr<ConfigurationOption> Option =
        std::make_unique<ConfigurationOption>(First, Second);
    Conf->addConfigurationOption(std::move(Option));
  }
  return Conf;
}

void Configuration::addConfigurationOption(
    std::shared_ptr<ConfigurationOption> Option) {
  this->OptionMappings[Option->getName()] = std::move(Option);
}

void Configuration::setConfigurationOption(const std::string &Name,
                                           const std::string &Value) {
  std::shared_ptr<ConfigurationOption> Option =
      std::make_unique<ConfigurationOption>(Name, Value);
  addConfigurationOption(std::move(Option));
}

std::string_view
Configuration::getConfigurationOptionValue(const std::string &Name) {
  auto Search = this->OptionMappings.find(Name);
  if (Search == this->OptionMappings.end()) {
    llvm::errs() << "The configuration option " << Name << " does not exist.\n";
    return "";
  }
  return Search->second->getValue();
}

std::vector<std::shared_ptr<ConfigurationOption>>
Configuration::getConfigurationOptions() {
  std::vector<std::shared_ptr<ConfigurationOption>> Options{};
  for (auto &OptionMapping : this->OptionMappings) {
    Options.insert(Options.begin(), OptionMapping.second);
  }
  return Options;
}

std::string Configuration::dumpToString() {
  llvm::json::Object Obj{};
  for (auto &Iterator : this->OptionMappings) {
    Obj[std::string(Iterator.first)] = std::string(Iterator.second->getValue());
  }
  llvm::json::Value Value(std::move(Obj));
  std::string DumpedString;
  llvm::raw_string_ostream Rs(DumpedString);
  Rs << Value;
  Rs.flush();
  return DumpedString;
}

} // namespace vara::feature