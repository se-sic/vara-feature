#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Configuration, parsingTest) {
  std::string ConfigurationString = R"({"baz":"1","foo":"true"})";
  auto Config =
      Configuration::createConfigurationFromString(ConfigurationString);
  EXPECT_TRUE(Config);
}

TEST(Configuration, negativeParsingTest) {
  std::string WrongConfigurationString = R"({"foo": "true"; "baz": "1"})";
  auto Config =
      Configuration::createConfigurationFromString(WrongConfigurationString);
  EXPECT_FALSE(Config);

  WrongConfigurationString = R"({"foo": ["0", "1"]})";
  Config =
      Configuration::createConfigurationFromString(WrongConfigurationString);
  EXPECT_FALSE(Config);

  WrongConfigurationString = R"(["0", "1"])";
  Config =
      Configuration::createConfigurationFromString(WrongConfigurationString);
  EXPECT_FALSE(Config);
}

TEST(Configuration, dumpTest) {
  std::string ConfigurationString = R"({"baz":"1","foo":"true"})";
  Configuration Config{};
  Config.setConfigurationOption("baz", "1");
  Config.setConfigurationOption("foo", "true");
  std::string Dump = Config.dumpToString();
  EXPECT_EQ(ConfigurationString, Dump);
  Config.setConfigurationOption("foo_baz", "2");
  EXPECT_EQ(R"({"baz":"1","foo":"true","foo_baz":"2"})", Config.dumpToString());
  EXPECT_EQ("2", Config.getConfigurationOptionValue("foo_baz"));
  EXPECT_EQ("", Config.getConfigurationOptionValue("a"));
  auto Configs = Config.getConfigurationOptions();
  EXPECT_EQ(3, Configs.size());
  EXPECT_EQ(R"({"baz":"1","foo":"true","foo_baz":"2"})", Config.dumpToString());
}
} // namespace vara::feature