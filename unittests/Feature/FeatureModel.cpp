#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModel, build) {
  FeatureModel::FeatureModelBuilder B;
  B.addChild("root", "a")->addFeature("a", true);
  EXPECT_TRUE(B.buildFeatureModel());
}

TEST(FeatureModel, size) {
  FeatureModel::FeatureModelBuilder B;
  B.addChild("root", "a")->addFeature("a", true);
  EXPECT_EQ(2, B.buildFeatureModel()->size());
}
} // namespace vara::feature
