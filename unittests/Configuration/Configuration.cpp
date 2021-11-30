#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Configuration, basicAccessors) {
  std::string configurationString = R"({"foo": "true", "baz": "1"})";
  Configuration config = Configuration::createConfigurationFromString(configurationString);
}
}