#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Configuration, basicAccessors) {
  std::string configurationString = R"({"foo": "true", "baz": "1"})";
  auto config = Configuration::createConfigurationFromString(configurationString);
  std::string dump = config->dumpToString();
}

TEST(Configuration, negativeTest) {
  std::string wrongConfigurationString = R"({"foo": "true"; "baz": "1"})";
  auto config = Configuration::createConfigurationFromString(wrongConfigurationString);

}
}