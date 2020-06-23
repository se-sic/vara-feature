#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModel, build) {
  FeatureModelBuilder B;

  B.addChild("root", "a")->addFeature("a", true);

  EXPECT_TRUE(B.buildFeatureModel());
}

TEST(FeatureModel, size) {
  FeatureModelBuilder B;

  B.addChild("root", "a")->addFeature("a", true);

  EXPECT_EQ(2, B.buildFeatureModel()->size());
}

TEST(FeatureModelBuilder, duplicate) {
  FeatureModelBuilder B;

  B.addChild("root", "a")->addFeature("a", true);

  EXPECT_FALSE(B.addFeature("a", true));
}

TEST(FeatureModel, addFeature) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"a", "aa"}, {"root", "aba"}, {"root", "a"}});
  assert(FM);

  FM->addFeature(std::make_unique<BinaryFeature>(
      "ab", false, std::nullopt, FM->getFeature("a"),
      Feature::FeatureSetType({FM->getFeature("aba")})));

  EXPECT_LT(*FM->getFeature("a"), *FM->getFeature("ab"));
  EXPECT_GT(*FM->getFeature("aba"), *FM->getFeature("ab"));
  EXPECT_EQ(*FM->getFeature("aba")->getParent(), *FM->getFeature("ab"));
}

TEST(FeatureModel, newRoot) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "b"}, {"root", "a"}});
  assert(FM);

  FM->addFeature(std::make_unique<BinaryFeature>(
      "new_root", false, std::nullopt, nullptr,
      Feature::FeatureSetType({FM->getFeature("root")})));

  EXPECT_TRUE(FM->getFeature("new_root")->isRoot());
  EXPECT_FALSE(FM->getFeature("root")->isRoot());
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("root"));
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("a"));
  EXPECT_LT(*FM->getFeature("new_root"), *FM->getFeature("b"));
  EXPECT_EQ(*FM->getFeature("new_root"), **FM->begin());
}
} // namespace vara::feature
