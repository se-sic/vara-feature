#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConfigurationOption, basicAccessors) {
  ConfigurationOption Option("foo", "true");
  EXPECT_EQ("foo", Option.name());
  EXPECT_TRUE(Option.isBool());
  EXPECT_FALSE(Option.isInt());
  EXPECT_FALSE(Option.isString());
  EXPECT_TRUE(Option.boolValue().getValue());
  EXPECT_EQ("true", Option.asString());
  EXPECT_EQ("foo: true", Option.toString());

  ConfigurationOption Option2("baz", "1");
  EXPECT_EQ("baz", Option2.name());
  EXPECT_TRUE(Option2.isInt());
  EXPECT_FALSE(Option2.isString());
  EXPECT_FALSE(Option2.isBool());
  EXPECT_EQ(1, Option2.intValue().getValue());
  EXPECT_EQ("1", Option2.asString());
  EXPECT_EQ("baz: 1", Option2.toString());

  ConfigurationOption UnparseableOption("foobar", "test");
  EXPECT_EQ("foobar", UnparseableOption.name());
  EXPECT_TRUE(UnparseableOption.isString());
  EXPECT_FALSE(UnparseableOption.isBool());
  EXPECT_FALSE(UnparseableOption.isInt());
  EXPECT_EQ("test", UnparseableOption.stringValue().getValue());
  EXPECT_EQ("test", UnparseableOption.asString());
  EXPECT_EQ("foobar: test", UnparseableOption.toString());
}

TEST(ConfigurationOption, negativeTests) {
  ConfigurationOption TrueOption("o", "false");
  EXPECT_FALSE(TrueOption.stringValue().hasValue());
  EXPECT_FALSE(TrueOption.intValue().hasValue());
  EXPECT_FALSE(TrueOption.boolValue().getValue());

  ConfigurationOption IntOption("p", "42");
  EXPECT_EQ(42, IntOption.intValue().getValue());
  EXPECT_FALSE(IntOption.stringValue().hasValue());
  EXPECT_FALSE(IntOption.boolValue().hasValue());

  ConfigurationOption StringOption("q", "test");
  EXPECT_EQ("test", StringOption.stringValue().getValue());
  EXPECT_FALSE(StringOption.intValue().hasValue());
  EXPECT_FALSE(StringOption.boolValue().hasValue());
}
} // namespace vara::feature