#include "vara/Configuration/Configuration.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConfigurationOption, basicAccessors) {
  ConfigurationOption Option("foo", "true");
  EXPECT_EQ("foo", Option.getName());
  EXPECT_EQ("true", Option.getValue());
  EXPECT_EQ("foo: true", Option.toString());

  ConfigurationOption Option2("baz", "1");
  EXPECT_EQ("baz", Option2.getName());
  EXPECT_EQ("1", Option2.getValue());
  EXPECT_EQ("baz: 1", Option2.toString());
}
} // namespace vara::feature