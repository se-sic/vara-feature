#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConfigurationOption, basicAccessors) {
  ConfigurationOption Option("foo", "true");
  EXPECT_EQ("foo", Option.getName());
  EXPECT_TRUE(Option.isBool());
  EXPECT_FALSE(Option.isInt());
  EXPECT_FALSE(Option.isString());
  EXPECT_TRUE(Option.getBoolValue());
  EXPECT_EQ("foo: true", Option.toString());

  ConfigurationOption Option2("baz", "1");
  EXPECT_EQ("baz", Option2.getName());
  EXPECT_TRUE(Option2.isInt());
  EXPECT_FALSE(Option2.isString());
  EXPECT_FALSE(Option2.isBool());
  EXPECT_EQ(1, Option2.getIntValue());
  EXPECT_EQ("baz: 1", Option2.toString());

  ConfigurationOption UnparseableOption("foobar", "test");
  EXPECT_EQ("foobar", UnparseableOption.getName());
  EXPECT_TRUE(UnparseableOption.isString());
  EXPECT_FALSE(UnparseableOption.isBool());
  EXPECT_FALSE(UnparseableOption.isInt());
  EXPECT_EQ("test", UnparseableOption.getStringValue());
  EXPECT_EQ("foobar: test", UnparseableOption.toString());
}

TEST(ConfigurationOption, negativeTests) {
  ConfigurationOption TrueOption("o", "false");
  EXPECT_EQ("", TrueOption.getStringValue());
  EXPECT_EQ(-1, TrueOption.getIntValue());
  EXPECT_FALSE(TrueOption.getBoolValue());

  ConfigurationOption IntOption("p", "42");
  EXPECT_EQ(42, IntOption.getIntValue());
  EXPECT_EQ("", IntOption.getStringValue());
  EXPECT_FALSE(IntOption.getBoolValue());

  ConfigurationOption StringOption("q", "test");
  EXPECT_EQ("test", StringOption.getStringValue());
  EXPECT_EQ(-1, StringOption.getIntValue());
  EXPECT_FALSE(StringOption.getBoolValue());
}
} // namespace vara::feature