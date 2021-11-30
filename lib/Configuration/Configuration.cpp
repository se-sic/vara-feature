#include "vara/Configuration/Configuration.h"

#include "llvm/Support/JSON.h"

namespace vara::feature {

static Configuration Configuration::createConfigurationFromString(std::string ConfigurationString) {
  Configuration configuration = new Configuration();
  // Read in the string using the json library

  //
}

void Configuration::addConfigurationOption(ConfigurationOption Option) {

}

void Configuration::setConfigurationOption(std::string Name, std::string Value) {

}

std::string Configuration::getConfigurationOptionValue(std::string Name) {
  return this->OptionMapping(Name);
}

std::vector<std::string> Configuration::getConfigurationOptions() {

}

std::string Configuration::dumpToString() {

}

}