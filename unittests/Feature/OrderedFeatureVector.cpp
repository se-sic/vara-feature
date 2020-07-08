#include "vara/Feature/FeatureModel.h"

#include "gtest/gtest.h"

namespace vara::feature {

TEST(OrderedFeatureVector, insert) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "a"}, {"root", "b"}});
  OrderedFeatureVector OFV;

  OFV.insert(FM->getFeature("a"));
  OFV.insert(FM->getFeature("root"));
  OFV.insert(FM->getFeature("b"));

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(*OFV.begin(), FM->getFeature("root"));
}

TEST(OrderedFeatureVector, insertFM) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "a"}, {"root", "b"}});
  OrderedFeatureVector OFV;

  OFV.insert(FM->features());

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(*OFV.begin(), FM->getFeature("root"));
}

TEST(OrderedFeatureVector, insertVariadic) {
  auto FM = FeatureModelBuilder().buildSimpleFeatureModel(
      {{"root", "a"}, {"root", "b"}});
  OrderedFeatureVector OFV;

  OFV.insert(FM->getFeature("a"), FM->getFeature("root"), FM->getFeature("b"));

  EXPECT_EQ(OFV.size(), FM->size());
  EXPECT_EQ(*OFV.begin(), FM->getFeature("root"));
}
} // namespace vara::feature
