#include "vara/Feature/FeatureModel.h"

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

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addExcludeConstraint) {
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
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addImplicationConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C = std::make_unique<ImpliesConstraint>(
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("a")),
      std::make_unique<PrimaryFeatureConstraint>(
          std::make_unique<Feature>("b")));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

// TODO(se-passau/VaRA#701): Replace string equals with subtree comparison
TEST(FeatureModelBuilder, addOrConstraint) {
  FeatureModelBuilder B;
  B.makeFeature<BinaryFeature>("a");
  B.makeFeature<BinaryFeature>("b");
  auto C =
      std::make_unique<OrConstraint>(std::make_unique<PrimaryFeatureConstraint>(
                                         std::make_unique<Feature>("a")),
                                     std::make_unique<PrimaryFeatureConstraint>(
                                         std::make_unique<Feature>("b")));
  auto Expected = C->toString();

  B.addConstraint(std::move(C));
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(
      (*FM->getFeature("a")->constraints().begin())->getRoot()->toString(),
      Expected);
}

TEST(FeatureModelBuilder, addBinaryFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  BinaryFeature A("a", false, {}, nullptr, CS);

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_BINARY);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}

TEST(FeatureModelBuilder, addNumericFeatureRef) {
  FeatureModelBuilder B;
  BinaryFeature AA("aa");
  auto CS = Feature::NodeSetType();
  CS.insert(&AA);
  NumericFeature A("a", std::vector<int>{1, 2, 3}, false, {}, nullptr,
                   CS);

  B.addFeature(AA);
  B.addParent("aa", "a")->addFeature(A);
  auto FM = B.buildFeatureModel();
  assert(FM);

  EXPECT_EQ(FM->getFeature("a")->getKind(), Feature::FeatureKind::FK_NUMERIC);
  EXPECT_TRUE(FM->getFeature("a")->hasEdgeTo(*FM->getFeature("aa")));
}
} // namespace vara::feature
