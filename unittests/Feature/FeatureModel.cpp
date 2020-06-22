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

TEST(FeatureModel, duplicate) {
  FeatureModelBuilder B;

  B.addChild("root", "a")->addFeature("a", true);

  EXPECT_FALSE(B.addFeature("a", true));
}
} // namespace vara::feature
