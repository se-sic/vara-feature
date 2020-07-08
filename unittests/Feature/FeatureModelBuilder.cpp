#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelBuilder, addBinaryFeature) {
  FeatureModelBuilder B;

  B.addFeature("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
}

TEST(FeatureModelBuilder, addNumericFeature) {
  FeatureModelBuilder B;

  B.addFeature("a", std::vector<int>{1, 2, 3});
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
}

TEST(FeatureModelBuilder, addOptionalFeature) {
  FeatureModelBuilder B;

  B.addFeature("a", true);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isOptional());
}

TEST(FeatureModelBuilder, addExclude) {
  FeatureModelBuilder B;

  B.addFeature("a");
  B.addExclude("a", "b")->addFeature("b");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("b")));
}

TEST(FeatureModelBuilder, addAlternative) {
  FeatureModelBuilder B;

  B.addFeature("a");
  B.addConstraint({{"a", true}, {"b", true}})->addFeature("b");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("b")));
  EXPECT_TRUE(FM->getFeature("b")->isAlternative(FM->getFeature("a")));
}

TEST(FeatureModelBuilder, addImplication) {
  FeatureModelBuilder B;

  B.addFeature("a");
  B.addConstraint({{"a", false}, {"b", true}})->addFeature("b");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("b")));
}

TEST(FeatureModelBuilder, duplicate) {
  FeatureModelBuilder B;

  B.addParent("a", "root")->addFeature("a", true);

  EXPECT_FALSE(B.addFeature("a", true));
}

TEST(FeatureModelBuilder, addBinaryFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature A("a", false, std::nullopt, nullptr, {&AA}, {&AA}, {&AA},
                  {&AA});

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->isChild(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, std::nullopt, nullptr,
                   {&AA}, {&AA}, {&AA}, {&AA});

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->isChild(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("aa")));
  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("aa")));
}
} // namespace vara::feature
