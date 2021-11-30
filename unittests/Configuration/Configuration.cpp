#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Configuration, basicAccessors) {
  std::string configurationString = R"({"baz":"1","foo":"true"})";
  auto config =
      Configuration::createConfigurationFromString(configurationString);
  EXPECT_TRUE(config);
  std::string dump = config->dumpToString();
  EXPECT_EQ(configurationString, dump);
  config->setConfigurationOption("foo_baz", "2");
  EXPECT_EQ(R"({"baz":"1","foo":"true","foo_baz":"2"})",
            config->dumpToString());
  EXPECT_EQ("2", config->getConfigurationOptionValue("foo_baz"));
  EXPECT_EQ("", config->getConfigurationOptionValue("a"));
  auto configs = config->getConfigurationOptions();
  EXPECT_EQ(3, configs.size());
  EXPECT_EQ(R"({"baz":"1","foo":"true","foo_baz":"2"})",
            config->dumpToString());
}

TEST(Configuration, negativeTest) {
  std::string wrongConfigurationString = R"({"foo": "true"; "baz": "1"})";
  auto config =
      Configuration::createConfigurationFromString(wrongConfigurationString);
  EXPECT_FALSE(config);

  wrongConfigurationString = R"({"foo": ["0", "1"]})";
  config =
      Configuration::createConfigurationFromString(wrongConfigurationString);
  EXPECT_FALSE(config);

  wrongConfigurationString = R"(["0", "1"])";
  config =
      Configuration::createConfigurationFromString(wrongConfigurationString);
  EXPECT_FALSE(config);
}
} // namespace vara::feature