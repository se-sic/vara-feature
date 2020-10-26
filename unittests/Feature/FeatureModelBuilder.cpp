#include <vara/Feature/FeatureModel.h>

#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelBuilder, addBinaryFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
}

TEST(FeatureModelBuilder, addNumericFeature) {
  FeatureModelBuilder B;

  B.makeFeature<NumericFeature>("a", std::vector<int>{1, 2, 3});
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
}

TEST(FeatureModelBuilder, addOptionalFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a", true);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_TRUE(FM->getFeature("a")->isOptional());
}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(FeatureModelBuilder, addExclude) {
//  FeatureModelBuilder B;
//
//  B.makeFeature<BinaryFeature>("a");
//  B.addExclude("a", "b")->makeFeature<BinaryFeature>("b");
//  auto FM = B.buildFeatureModel();
//  assert(FM);
//
//  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("b")));
//}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(FeatureModelBuilder, addAlternative) {
//  FeatureModelBuilder B;
//
//  B.makeFeature<BinaryFeature>("a");
//  B.addConstraint({{"a", true}, {"b",
//  true}})->makeFeature<BinaryFeature>("b"); auto FM = B.buildFeatureModel();
//  assert(FM);
//
//  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("b")));
//  EXPECT_TRUE(FM->getFeature("b")->isAlternative(FM->getFeature("a")));
//}

// TODO(s9latimm): Refactor with new Constraints representation
// TEST(FeatureModelBuilder, addImplication) {
//  FeatureModelBuilder B;
//
//  B.makeFeature<BinaryFeature>("a");
//  B.addConstraint({{"a", false}, {"b",
//  true}})->makeFeature<BinaryFeature>("b"); auto FM = B.buildFeatureModel();
//  assert(FM);
//
//  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("b")));
//}

TEST(FeatureModelBuilder, duplicate) {
  FeatureModelBuilder B;

  B.addParent("a", "root")->makeFeature<BinaryFeature>("a", true);

  EXPECT_FALSE(B.makeFeature<BinaryFeature>("a", true));
}

TEST(FeatureModelBuilder, addBinaryFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  BinaryFeature A("a", false, std::nullopt, nullptr, {&AA});

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  // TODO(s9latimm): Refactor with new Constraints representation
  //  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("aa")));
  //  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("aa")));
  //  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, std::nullopt, nullptr,
                   {&AA});

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
  // TODO(s9latimm): Refactor with new Constraints representation
  //  EXPECT_TRUE(FM->getFeature("a")->isExcluded(FM->getFeature("aa")));
  //  EXPECT_TRUE(FM->getFeature("a")->isImplied(FM->getFeature("aa")));
  //  EXPECT_TRUE(FM->getFeature("a")->isAlternative(FM->getFeature("aa")));
}
} // namespace vara::feature
