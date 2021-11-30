#include "vara/Configuration/ConfigurationOption.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(ConfigurationOption, basicAccessors) {
  ConfigurationOption option("foo", "true");
  EXPECT_EQ("foo", option.getName());
  EXPECT_EQ("true", option.getValue());
  EXPECT_EQ("foo: true", option.toString());

  ConfigurationOption option2("baz", "1");
  EXPECT_EQ("baz", option2.getName());
  EXPECT_EQ("1", option2.getValue());
  EXPECT_EQ("baz: 1", option2.toString());
}
}