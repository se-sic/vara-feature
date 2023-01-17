#include "vara/Feature/FeatureModelBuilder.h"

#include "Utils/UnittestHelper.h"
#include "gtest/gtest.h"

namespace vara::feature {

TEST(FeatureModelBuilder, addBinaryFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
}

TEST(FeatureModelBuilder, duplicateFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();

  EXPECT_FALSE(FM);
}

TEST(FeatureModelBuilder, addNumericFeature) {
  FeatureModelBuilder B;

  B.makeFeature<NumericFeature>("a", NumericFeature::ValueListType{1, 2, 3});
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
}

TEST(FeatureModelBuilder, addOptionalFeature) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("a", true);
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_TRUE(FM->getFeature("a")->isOptional());
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addImpliedExcludeConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = std::make_unique<ImpliesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("a")),
      std::make_unique<NotConstraint>(
          std::make_unique<PrimaryFeatureConstraint>(
              std::make_unique<Feature>("b"))));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));

  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addImpliesConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = createBinaryConstraint<ImpliesConstraint>("a", "b");
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addOrConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = createBinaryConstraint<OrConstraint>("a", "b");
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

TEST(FeatureModelBuilder, addBinaryFeatureRef) {
  FeatureModelBuilder B;

  B.makeFeature<BinaryFeature>("aa");
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("a");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addMultipleFeaturesWithEdges) {
  FeatureModelBuilder B;

  B.makeRoot("newRoot");
  B.makeFeature<BinaryFeature>("a", true);
  B.makeFeature<BinaryFeature>("b", true);
  B.makeFeature<BinaryFeature>("c", true);

  B.addEdge("newRoot", "a");
  B.addEdge("newRoot", "b");
  B.addEdge("newRoot", "c");

  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_TRUE(FM->getFeature("a") != nullptr);
  EXPECT_TRUE(FM->getFeature("b") != nullptr);
  EXPECT_TRUE(FM->getFeature("c") != nullptr);

  EXPECT_TRUE(FM->getRoot() != nullptr);
  EXPECT_EQ(FM->getRoot(), FM->getFeature("newRoot"));

  EXPECT_TRUE(FM->getRoot()->hasEdgeTo(*FM->getFeature("a")));
  EXPECT_TRUE(FM->getRoot()->hasEdgeTo(*FM->getFeature("b")));
  EXPECT_TRUE(FM->getRoot()->hasEdgeTo(*FM->getFeature("c")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;

  B.makeFeature<NumericFeature>("a", NumericFeature::ValueListType{1, 2, 3});
  B.addEdge("a", "aa")->makeFeature<BinaryFeature>("aa");
  auto FM = B.buildFeatureModel();
  ASSERT_TRUE(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

} // namespace vara::feature
