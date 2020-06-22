#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {
TEST(Feature, equal) {
  BinaryFeature A0("A");
  BinaryFeature A1("A");
  BinaryFeature A2("a");
  BinaryFeature B("B");

  EXPECT_TRUE(A0 == A1);
  EXPECT_TRUE(A0 == A2);
  EXPECT_FALSE(A0 == B);
}

TEST(Feature, lt) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("root"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("aa"));
  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("b"));
  EXPECT_LT(*FM->getFeature("aa"), *FM->getFeature("b"));
}

TEST(Feature, gt) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "b"}, {"root", "a"}});
  assert(FM);

  EXPECT_GT(*FM->getFeature("a"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("aa"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("root"));
  EXPECT_GT(*FM->getFeature("aa"), *FM->getFeature("a"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("a"));
  EXPECT_GT(*FM->getFeature("b"), *FM->getFeature("aa"));
}
} // namespace vara::feature
