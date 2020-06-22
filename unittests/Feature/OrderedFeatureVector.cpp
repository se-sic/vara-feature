#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(OrderedFeatureVector, sort) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel({{"root", "a"},
                                                           {"b", "bb"},
                                                           {"aa", "aaa"},
                                                           {"root", "b"},
                                                           {"a", "aa"},
                                                           {"a", "ab"}});
  assert(FM);

  auto It = FM->begin();

  EXPECT_EQ("root", (*It++)->getName());
  EXPECT_EQ("a", (*It++)->getName());
  EXPECT_EQ("aa", (*It++)->getName());
  EXPECT_EQ("aaa", (*It++)->getName());
  EXPECT_EQ("ab", (*It++)->getName());
  EXPECT_EQ("b", (*It++)->getName());
  EXPECT_EQ("bb", (*It++)->getName());
  EXPECT_EQ(FM->end(), It);
}
} // namespace vara::feature
